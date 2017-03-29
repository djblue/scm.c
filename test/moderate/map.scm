(define (map fn ls)
  (if (null? ls)
    ls
    (cons (fn (car ls)) (map fn (cdr ls)))))

(write (map (lambda (x) x) '(1 2 3 4)))

