(load "lib.scm")

(define (lookup access e)
  (recur nxtrib ([e e] [rib (car access)])
    (if (= rib 0)
      (recur nxtelt ([r (car e)] [elt (cdr access)])
         (if (= elt 0)
           r
           (nxtelt (cdr r) (- elt 1))))
      (nxtrib (cdr e) (- rib 1)))))

(define (closure body e) (list body e))

(define (continuation s)
  (closure (list 'nuate s '(0 . 0)) '()))

(define (call-frame x e r s)
  (list x e r s))

(define (extend e r) (cons r e))

; vm registers
; a: the accumulator
; x: the next expression
; e: the current environment
; r: the current value rib
; s: the current stack

(define (VM a x e r s)
  #;(for-each display
    (list
      (list 'a a)
      (list 'x x)
      (list 'e e)
      (list 'r r)
      (list 's s)
      'call-frame))
  (record-case x
    [halt () a]
    [refer (var x)
      (VM (car (lookup var e)) x e r s)]
    [constant (obj x)
      (VM obj x e r s)]
    [close (body x)
      (VM (closure body e) x e r s)]
    [test (then else)
      (VM a (if a then else) e r s)]
    [assign (var x)
      (begin
        (set-car! (lookup var e) a)
        (VM a x e r s))]
    [conti (x)
      (VM (continuation s) x e r s)]
    [nuate (s var)
      (VM (car (lookup var e)) '(return) e r s)]
    [frame (ret x)
      (VM a x e '() (call-frame ret e r s))]
    [argument (x)
      (VM a x e (cons a r) s)]
    [apply ()
      (if (procedure? a)
        (VM (apply a r) '(return) e '() s)
        (record a (body e)
          (VM a body (extend e r) '() s)))]
    [return ()
      (record s (x e r s)
        (VM a x e r s))]))

