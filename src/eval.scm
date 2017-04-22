(load "compile.scm")
(load "vm.scm")

(define (evaluate x)
  (VM '() (compile x '(halt)) '() '() '()))
