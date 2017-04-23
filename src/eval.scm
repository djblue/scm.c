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

  (define vars '((range = cons -)))
  (define vals (list (list '() = cons -)))

  (define program
    (compile
      '(set! range (lambda (n ls i)
                     (if (= i 0)
                       ls
                       (range n (cons (- i 1) ls) (- i 1)))))
      vars
      (compile '(range 10 '() 10) vars '(halt))))

  (VM '() program vals '() '())

  )
