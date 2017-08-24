(load "lib.scm")

(load "set.scm")

(define (tail? next)
  (eq? (car next) 'return))

(define (macro-form? form)
  (and (pair? form)
       (symbol? (car form))
       (set-member? (car form) (caadr (interaction-environment)))
       (macro? (eval (car form)))))

(define (macroexpand-1 form)
  (if (macro-form? form)
      (apply (eval (car form)) (cdr form))
      form))

(define (macroexpand form)
  (if (macro-form? form)
      (macroexpand (macroexpand-1 form))
      form))

(define (macroexpand-all form)
  (let ([expanded (macroexpand form)])
    (if (pair? expanded)
        (map macroexpand-all expanded)
        expanded)))

(define (asm bytecode)
  (if (and (pair? bytecode)  (symbol? (car bytecode)))
    (cons
      (let ([code (car bytecode)])
        (cond
          [(= code 'halt)           0]
          [(= code 'refer-local)    1]
          [(= code 'refer-free)     2]
          [(= code 'refer-global)   3]
          [(= code 'indirect)       4]
          [(= code 'constant)       5]
          [(= code 'close)          6]
          [(= code 'box)            7]
          [(= code 'test)           8]
          [(= code 'assign-local)   9]
          [(= code 'assign-free)   10]
          [(= code 'assign-global) 11]
          [(= code 'conti)         12]
          [(= code 'nuate)         13]
          [(= code 'frame)         14]
          [(= code 'argument)      15]
          [(= code 'shift)         16]
          [(= code 'apply)         17]
          [(= code 'return)        18]
          else (error "asm: unknown opcode" code)))
      (map asm (cdr bytecode)))
    bytecode))

(define (compile-lookup x e return-local return-free return-global)
  (recur nxtlocal ([locals (if (null? e) '() (car e))] [n 0])
    (if (null? locals)
        (recur nxtfree ([free (if (null? e) '() (cdr e))] [n 0])
           (cond
             [(null? free) (return-global x)]
             [(eq? (car free) x) (return-free n)]
             [else (nxtfree (cdr free) (+ n 1))]))
        (if (eq? (car locals) x)
            (return-local n)
            (nxtlocal (cdr locals) (+ n 1))))))

(define (compile-refer x e next)
  (compile-lookup x e
    (lambda (n) (list 'refer-local n next))
    (lambda (n) (list 'refer-free n next))
    (lambda (sym) (list 'refer-global sym next))))

; finds the set of free variables
; x - expression
; b - initial set of bound variables
(define (find-free x b)
  (cond
    [(symbol? x) (if (set-member? x b) '() (list x))]
    [(pair? x)
     (record-case x
        [quote (obj) '()]
        [lambda (vars body)
          (find-free body (set-union vars b))]
        [if (test then else)
          (set-union (find-free test b)
                     (set-union
                       (find-free then b)
                       (find-free else b)))]
        [set! (var exp)
          (set-union (if (set-member? var b) '() (list var))
                     (find-free exp b))]
        [call/cc (exp) (find-free exp b)]
        [else
          (recur next ([x x])
             (if (null? x)
                 '()
                 (set-union (find-free (car x) b)
                            (next (cdr x)))))])]
    [else '()]))

(define (find-sets x v)
  (cond
    [(symbol? x) '()]
    [(pair? x)
     (record-case x
        [quote (obj) '()]
        [lambda (vars body)
          (find-sets body (set-minus v vars))]
        [if (test then else)
          (set-union (find-sets test v)
                     (set-union (find-sets then v)
                                (find-sets else v)))]
        [set! (var x)
          (set-union (if (set-member? var v) (list var) '())
                     (find-sets x v))]
        [call/cc (exp) (find-sets exp v)]
        [else
          (recur next ([x x])
            (if (null? x)
                '()
                (set-union (find-sets (car x) v)
                           (next (cdr x)))))])]
    [else '()]))

(define (make-boxes sets vars next)
  (recur f ([vars vars] [n 0])
    (if (null? vars)
        next
        (if (set-member? (car vars) sets)
            (list 'box n (f (cdr vars) (+ n 1)))
            (f (cdr vars) (+ n 1))))))

(define (collect-free vars e next)
  (if (null? vars)
    next
    (collect-free (cdr vars) e
      (compile-refer (car vars) e
                     (list 'argument next)))))

(define (compile x e s next)
  (cond
    [(symbol? x)
     (compile-refer x e
        (if (set-member? x s)
          (list 'indirect next)
          next))]
    [(pair? x)
     (record-case x
       [quote (obj) (list 'constant obj next)]
       [lambda (vars body)
         (let ([free (set-intersect (find-free body vars)
                                    (if (null? e) '()
                                        (set-union (car e)
                                                   (cdr e))))]
               [sets (find-sets body vars)])
           (collect-free free e
              (list 'close
                    (length free)
                    (make-boxes sets vars
                      (compile body
                               (cons (reverse vars) free)
                               (set-union
                                 sets
                                 (set-intersect s free))
                               (list 'return (length vars))))
                    next)))]
       [if (test then else)
         (let ([thenc (compile then e s next)]
               [elsec (compile else e s next)])
           (compile test e s (list 'test thenc elsec)))]
       [set! (var x)
         (compile-lookup var e
            (lambda (n)
              (compile x e s (list 'assign-local n next)))
            (lambda (n)
              (compile x e s (list 'assign-free n next)))
            (lambda (sym)
              (compile x e s (list 'assign-global sym next))))]
       [define (sym x)
        (compile x e s (list 'assign-global sym next))]
       [call/cc (x)
        (let ([c (list 'conti
                       (list 'argument
                             (compile x e s
                                (if (tail? next)
                                  (list 'shift
                                        1
                                        (cadr next)
                                        '(apply 1))
                                  '(apply 1)))))])
          (if (tail? next)
              c
              (list 'frame next c)))]
       [else
         (recur loop ([args (reverse (cdr x))]
                      [c (compile (car x) e s
                            (if (tail? next)
                                (list 'shift
                                      (length (cdr x))
                                      (cadr next)
                                      (list 'apply (length (cdr x))))
                                (list 'apply (length (cdr x))))
                           (list 'apply (length (cdr x))))])
                (if (null? args)
                  (if (tail? next)
                    c
                    (list 'frame next c))
                  (loop (cdr args)
                        (compile (car args)
                                 e
                                 s
                                 (list 'argument c)))))])]
    [else
      (list 'constant x next)]))
