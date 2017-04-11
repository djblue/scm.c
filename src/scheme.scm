; evaluate a given expression in a specific environment
(define (evaluate e env)
  (if (atom? e)
    (cond
      [(symbol? e) (lookup e env)]
      [(or (number? e)
           (string? e)
           (char? e)
           (boolean? e)) e]
      [else (error "Cannot evaluate" e)])
    (case (car e)
      [(quote)  (cadr e)]
      [(if)     (if (evaluate (cadr e) env)
                    (evaluate (caddr e) env)
                    (evaluate (cadddr e) env))]
      [(begin)  (eprogn (cdr e) env)]
      [(set!)   (update! (cadr e) env (evaluate (caddr e) env))]
      [(lambda) (make-function (cadr e) (cddr e) env)]
      [else     (invoke (evaluate (car e) env)
                        (evlis (cdr e) env))])))

; determine if `e` is atomic
(define (atom? e)
  (or (symbol? e)
      (number? e)
      (string? e)
      (char? e)
      (boolean? e)))

; lookup a variable in an environment
(define (lookup id env)
  (if (pair? env)
      (if (eq? (caar env) id)
          (cdar env)
          (lookup id (cdr env)))
      (error "No such binding" id)))

; update a variable in an environment
(define (update! id env value)
  (if (pair? env)
      (if (eq? (caar env) id)
          (begin (set-cdr! (car env) value)
                 value)
          (update! id (cdr env) value))
      (error "No such binding" id)))

(define (make-function variables body env)
  (lambda (values)
    (eprogn body (extend env variables values))))

(define (invoke fn args)
  (if (procedure? fn)
      (fn args)
      (error "Not a function" fn)))

; evaluate a sequence of forms sequentially
(define (eprogn exps env)
  (if (pair? (car exps))
      (if (pair? (cdr exps))
          (begin (evaluate (car exps) env)
                 (eprogn (cdr exps) env))
          (evaluate (car exps) env))
      '()))

; evaluates a list of expressions and returns the
; corresponding list of values of those expressions.
(define (evlis exps env)
  (if (pair? exps)
    (cons (evaluate (car exps) env)
          (evlis (cdr exps) env))
    '()))

; extend an environment with new variables bindings
(define (extend env variables values)
  (cond
    [(pair? variables)
     (if (pair? values)
         (cons (cons (car variables) (car values))
               (extend env (cdr variables) (cdr values)))
         (error "Too less values"))]
    [(null? variables)
     (if (null? values)
         env
         (error "Too much values"))]
    [(symbol? variables) (cons (cons variables values) env)]))

(define env.init '())
(define env.global env.init)

(define (chapter1-scheme)
  (define (toplevel)
    (display (evaluate (read) env.global))
    (toplevel))
  (toplevel))

