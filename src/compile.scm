(load "lib.scm")

(define HALT 0)  
(define REFER 1)
(define CONSTANT 2)
(define CLOSE 3)
(define TEST 4)
(define ASSIGN 5)
(define CONTI 6)
(define NUATE 7)
(define FRAME 8)
(define ARGUMENT 9)
(define APPLY 10)
(define RETURN 11)

(define (tail? next)
  (eq? (car next) RETURN))

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
    [(symbol? x) (list REFER (compile-lookup x e) next)]
    [(pair? x)
     (record-case x
       [quote (obj)
        (list CONSTANT obj next)]
       [lambda (vars body)
        (list CLOSE (compile body
                             (extend e vars)
                             (list RETURN)) next)]
       [if (test then else)
         (let ([thenc (compile then e next)]
               [elsec (compile else e next)])
           (compile test e (list TEST thenc elsec)))]
       [set! (var x)
         (let ([access (compile-lookup var e)])
            (compile x e (list ASSIGN access next)))]
       [call/cc
         (let ([c (list CONTI
                        (list ARGUMENT 
                              (compile x e (list APPLY))))])
           (if (tail? next)
               c
               (list FRAME next c)))]
       [else
         (recur loop ([args (cdr x)]
                      [c (compile (car x) e (list APPLY))])
                (if (null? args)
                  (if (tail? next)
                    c
                    (list FRAME next c))
                  (loop (cdr args)
                        (compile (car args)
                                 e
                                 (list ARGUMENT c)))))])]
    [else
      (list CONSTANT x next)]))
