(load "lib.scm")

(define (closure body n s)
  (let ([v (make-vector (+ n 1))])
    (vector-set! v 0 body)
    (recur f ([i 0])
      (if (not (= i n))
        (begin
          (vector-set! v (+ i 1) (index s i))
          (f (+ i 1)))))
    v))

(define (closure-body c)
  (vector-ref c 0))

(define (index-closure c n)
  (vector-ref c (+ n 1)))

(define (save-stack s)
  (let ([v (make-vector s)])
    (recur copy ([i 0])
      (if (not (= i s))
        (begin
          (vector-set! v i (vector-ref stack i))
          (copy (+ i 1)))))
    v))

(define (restore-stack v)
  (let ([s (vector-length v)])
    (recur copy ([i 0])
      (if (not (= i s))
        (begin
          (vector-set! stack i (vector-ref v i))
          (copy (+ i 1)))))
    s))

(define (continuation s)
  (closure
    (list 'refer 0 0 (list 'nuate (save-stack s) '(return 0)))
    '()))

(define (push x s)
  (vector-set! stack s x)
  (+ s 1))

(define (index s i)
  (vector-ref stack (- (- s i) 1)))

(define (index-set! s i v)
  (vector-set! stack (- (- s i) 1) v))

(define (stack-args s n ls)
  (if (= n 0)
    ls
    (stack-args s (- n 1) (cons (index s n) ls))))

; creates a single-cell box containing obj
(define (box obj) (cons 'BOX obj))

; returns the contents of box
(define (unbox box) (cdr box))

; stores obj in box
(define (set-box! box obj) (set-cdr! box obj))

(define (shift-args n m s)
  (recur nxtarg ([i (- n 1)])
     (if (not (< i 0))
       (begin
         (index-set! s (+ i m) (index s i))
         (nxtarg (- i 1)))))
  (- s m))

; vm registers
; a - the accumulator
; x - the next expression
; f - the current frame pointer
; c - the current closure
; s - the current stack pointer

(define (VM a x f c s)
  (comment for-each display
    (list
      (list 'a a)
      (list 'x x)
      (list 'f f)
      (list 'c c)
      (list 's s)
      '=============call-frame=============))
  (record-case x
    [halt () a]
    [refer-local (n x)
      (VM (index f n) x f c s)]
    [refer-free (n x)
      (VM (index-closure c n) x f c s)]
    [refer-global (sym x)
      (VM (eval sym) x f c s)]
    [indirect (x)
      (VM (unbox a) x f c s)]
    [constant (obj x)
      (VM obj x f c s)]
    [close (n body x)
      (VM (closure body n s) x f c (- s n))]
    [box (n x)
      (begin
        (index-set! s n (box (index s n)))
        (VM a x f c s))]
    [test (then else)
      (VM a (if a then else) f c s)]
    [assign-local (n x)
      (begin
        (set-box! (index f n) a)
        (VM a x f c s))]
    [assign-free (n x)
      (begin
        (set-box! (index-closure c n) a)
        (VM a x f c s))]
    [conti (x)
      (VM (continuation s) x f c s)]
    [naute (stack x)
      (VM a x f c (restore-stack stack))]
    [frame (ret x)
      (VM a x f c (push ret (push f (push c s))))]
    [argument (x)
      (VM a x f c (push a s))]
    [shift (n m x)
      (VM a x f c (shift-args n m s))]
    [apply (n)
      (if (procedure? a)
        (VM
          (apply a (stack-args (+ s 1) n))
          (list 'return n) f c s)
        (VM a (closure-body a) s a s))]
    [return (n)
      (let ([s (- s n)])
        (VM a (index s 0) (index s 1) (index s 2) (- s 3)))]))

