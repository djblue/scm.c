(write (pair? (cons 1 2)))
(write (car (cons 1 2)))
(write (cdr (cons 1 2)))

(write (pair? '(1 . 2)))
(write (car '(1 . 2)))
(write (cdr '(1 . 2)))

(define p (cons '() '()))

(write (car p))
(set-car! p 1)
(write (car p))

(write (cdr p))
(set-cdr! p 2)
(write (cdr p))
