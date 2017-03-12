; pairs
(define (caar p) (car (car p)))
(define (cadr p) (car (cdr p)))
(define (cdar p) (cdr (car p)))
(define (cddr p) (cdr (cdr p)))

(define (caaar p) (car (caar p)))
(define (caadr p) (car (cadr p)))
(define (cadar p) (car (cdar p)))
(define (caddr p) (car (cddr p)))
(define (cdaar p) (cdr (caar p)))
(define (cdadr p) (cdr (cadr p)))
(define (cddar p) (cdr (cdar p)))
(define (cdddr p) (cdr (cddr p)))

(define (caaaar p) (car (caaar p)))
(define (caaadr p) (car (caadr p)))
(define (caadar p) (car (cadar p)))
(define (caaddr p) (car (caddr p)))
(define (cadaar p) (car (cdaar p)))
(define (cadadr p) (car (cdadr p)))
(define (caddar p) (car (cddar p)))
(define (cadddr p) (car (cdddr p)))
(define (cdaaar p) (cdr (caaar p)))
(define (cdaadr p) (cdr (caadr p)))
(define (cdadar p) (cdr (cadar p)))
(define (cdaddr p) (cdr (caddr p)))
(define (cddaar p) (cdr (cdaar p)))
(define (cddadr p) (cdr (cdadr p)))
(define (cdddar p) (cdr (cddar p)))
(define (cddddr p) (cdr (cdddr p)))

(define first   car)
(define rest    cdr)
(define second  cadr)
(define third   caddr)
(define fourth  cadddr)

; reduce a list into a single value
(define (reduce fn ls acc)
  (if (null? ls)
    acc
    (reduce fn (cdr ls) (fn acc (car ls)))))

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

(define (length ls)
  (define (iter ls n)
    (if (null? ls)
      n
      (iter (cdr ls) (inc n))))
  (iter ls 0))

(define (zero? n) (= n 0))

(define (range n)
  (define (range-iter ls i)
    (if (= i n)
      ls
      (range-iter (cons i ls) (inc i))))
  (reverse (range-iter '() 0)))

(define list (lambda ls ls))

