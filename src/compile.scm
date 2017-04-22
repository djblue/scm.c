(load "lib.scm")

(define (tail? next)
  (eq? (car next) 'return))

(define (compile x next)
  (cond
    [(symbol? x) (list 'refer x next)]
    [(pair? x)
     (record-case x
       [quote (obj)
        (list 'constant obj next)]
       [lambda (vars body)
        (list 'close vars (compile body '(return)) next)]
       [if (test then else)
         (let ([thenc (compile then next)]
               [elsec (compile else next)])
           (compile test (list 'test thenc elsec)))]
       [set! (var x)
         (compile x (list 'assign var next))]
       [call/cc
         (let ([c (list 'conti
                        (list 'argument 
                              (compile x '(apply))))])
           (if (tail? next)
               c
               (list 'frame next c)))]
       [else
         (recur loop ([args (cdr x)]
                      [c (compile (car x) '(apply))])
                (if (null? args)
                  (if (tail? next)
                    c
                    (list 'frame next c))
                  (loop (cdr args)
                        (compile (car args)
                                 (list 'argument c)))))])]
    [else
      (list 'constant x next)]))
