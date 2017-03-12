(define (reverse ls)
  (define (reverse-inner ls acc)
    (if (null? ls)
      acc
      (reverse-inner (cdr ls) (cons (car ls) acc))))
  (reverse-inner ls '()))

(define (inc x) (+ x 1))

(define (range n)
  (define (range-iter ls i)
    (if (= i n)
      ls
      (range-iter (cons i ls) (inc i))))
  (reverse (range-iter '() 0)))

(write (range 10))
