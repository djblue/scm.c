; map a function over a list
(define (map fn ls)
  (if (null? ls)
        ls ; base case
        (cons
          (fn (car ls))
          (map fn (cdr ls)))))

; filter items from a list
(define (filter fn ls)
  (if (null? ls)
    ls
    (if (fn (car ls))
      (cons (car ls) (filter fn (cdr ls)))
      (filter fn (cdr ls)))))

; reverse a list
(define (reverse ls)
  (define (reverse-inner ls acc)
    (if (null? ls)
      acc
      (reverse-inner (cdr ls) (cons (car ls) acc))))
  (reverse-inner ls '()))

(define (concat ls value)
  (if (null? ls)
    value
    (cons (car ls) (concat (cdr ls) value))))

(define (append ls value)
  (concat ls (cons value '())))

(define (inc x) (+ x 1))

; invert a boolean value
(define (not x) (if x #f #t))

(define (count ls)
  (define (count-iter ls n)
    (if (null? ls)
      n
      (count-iter (cdr ls) (inc n))))
  (count-iter ls 0))

(define (zero? n) (= n 0))

(define (range n)
  (define (range-iter ls i)
    (if (= i n)
      ls
      (range-iter (cons i ls) (inc i))))
  (reverse (range-iter '() 0)))

(define list (lambda ls ls))

