(load "compile.scm")
(load "vm.scm")

(define (evaluate x)
  (VM '() (compile x '(halt)) (interaction-environment) '() '()))

(define env (extend '() '(+) (list +)))

(define (evaluate x)
  (VM '() (compile x '(halt)) env '()))

(define program '(define (range n)
  (define (iter ls i)
    (if (= i 0)
      ls
      (iter (cons (- i 1) ls) (- i 1))))
  (iter '() n)))

; vm registers
; a: the accumulator
; x: the next expression
; e: the current environment
; r: the current value rib
; s: the current stack

(evaluate '(+ 1 2))
