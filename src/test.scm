;(load "compile.scm")
;(define vars '((range = cons -)))


(comment
(define program
  (compile
    '(set! range (lambda (n ls i)
                   (if (= i 0)
                     ls
                     (range n (cons (- i 1) ls) (- i 1)))))
    vars
    (compile '(range 100000 '() 100000) vars (list HALT))))
  )

(define vals (list (list range = cons -)))

(define program '(3  (8  (4  (1  (0 . 1)  (11))  (8  (9  (8  (9  (1  (0 . 0)  (9  (1  (1 . 0)  (10)))))  (1  (0 . 1)  (9  (8  (9  (1  (1 . 2)  (10)))  (2 1  (9  (1  (0 . 2)  (9  (1  (1 . 3)  (10)))))))))))  (2 1  (9  (1  (0 . 2)  (9  (1  (1 . 3)  (10))))))))  (2 0  (9  (1  (0 . 2)  (9  (1  (1 . 1)  (10)))))))  (5  (0 . 0)  (8  (0)  (2 100000  (9  (2  ()  (9  (2 100000  (9  (1  (0 . 0)  (10))))))))))))
 
(beval program vals)
