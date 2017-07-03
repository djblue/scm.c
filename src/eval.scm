(load "compile.scm")

(load "vm.scm")

(define (evaluate x)
  (VM '()
    (compile x (map car (interaction-environment)) '(halt))
    (map cdr (interaction-environment))
    '()
    '()))

(define (repl)
  (display 'repl>)
  (display (evaluate (read)))
  (repl))

(comment

  (load "eval.scm")

  (load "compile.scm")

  (define program (compile '(cons 1 2) vars (list HALT)))

  (define vars '((range = cons -)))
  (define vals (list (list range = cons -)))

  (define program
    (compile
      '(set! range (lambda (n ls i)
                     (if (= i 0)
                       ls
                       (range n (cons (- i 1) ls) (- i 1)))))
      vars
      (compile '(range 100000 '() 100000) vars (list HALT))))

  (beval program vals)

  (VM '() program vals '() '())

  )
