(load "lib.scm")

(define (tail? next)
  (eq? (car next) 'return))

(define (extend e r) (cons r e))

(define (compile-lookup var e)
  (recur nxtrib ([e e] [rib 0])
     (recur nxtelt ([vars (car e)] [elt 0])
       (cond
         [(null? vars)
          (if (null? (cdr e))
            (error "compile-lookup: no such binding" var)
            (nxtrib (cdr e) (+ rib 1)))]
         [(eq? (car vars) var) (cons rib elt)]
         [else (nxtelt (cdr vars) (+ elt 1))]))))

(define (compile x e next)
  (cond
    [(symbol? x) (list 'refer (compile-lookup x e) next)]
    [(pair? x)
     (record-case x
       [quote (obj)
        (list 'constant obj next)]
       [lambda (vars body)
        (list 'close (compile body (extend e vars)  '(return)) next)]
       [if (test then else)
         (let ([thenc (compile then e next)]
               [elsec (compile else e next)])
           (compile test e (list 'test thenc elsec)))]
       [set! (var x)
         (let ([access (compile-lookup var e)])
            (compile x e (list 'assign access next)))]
       [call/cc
         (let ([c (list 'conti
                        (list 'argument 
                              (compile x e '(apply))))])
           (if (tail? next)
               c
               (list 'frame next c)))]
       [else
         (recur loop ([args (cdr x)]
                      [c (compile (car x) e '(apply))])
                (if (null? args)
                  (if (tail? next)
                    c
                    (list 'frame next c))
                  (loop (cdr args)
                        (compile (car args)
                                 e
                                 (list 'argument c)))))])]
    [else
      (list 'constant x next)]))
