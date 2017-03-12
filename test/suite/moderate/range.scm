(define reverse (lambda (ls)
  (define reverse-inner (lambda (ls acc)
    (if (null? ls)
      acc
      (reverse-inner (cdr ls) (cons (car ls) acc)))))
  (reverse-inner ls '())))

(define inc (lambda (x) (+ x 1)))

(define range (lambda (n)
  (define range-iter (lambda (ls i)
    (if (= i n)
      ls
      (range-iter (cons i ls) (inc i)))))
  (reverse (range-iter '() 0))))

(write (range 10))
