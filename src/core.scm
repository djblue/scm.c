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

; display an object and then return it
(define (display o) (write o) o)

; reduce a list into a single value
(define (reduce fn ls acc)
  (if (null? ls)
    acc
    (reduce fn (cdr ls) (fn acc (car ls)))))

(define (member v ls)
  (cond
    [(null? ls) #f]
    [(= v (car ls)) ls]
    [else (member v (cdr ls))]))

; map a function over a list
(define (map fn ls)
  (define (iter ls acc)
    (if (null? ls)
      acc
      (iter (cdr ls) (cons (fn (car ls)) acc))))
  (reverse (iter ls '())))

; apply fn to each element in a list
(define (for-each fn ls)
  (if (null? ls)
    '()
    (begin
      (fn (car ls))
      (for-each fn (cdr ls)))))

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

(define (append . args)
  (if (null? args)
    '()
    (if (null? (car args))
        (apply append (cdr args))
        (cons (caar args)
              (apply append (cons (cdar args) (cdr args)))))))

(define (inc x) (+ x 1))

(define (dec x) (- x 1))

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
  (define (iter ls i)
    (if (= i 0)
      ls
      (iter (cons (- i 1) ls) (- i 1))))
  (iter '() n))

(define list (lambda ls ls))

(define (id x) x)

(define (compose f g)
  (lambda args
    (f (apply g args))))

(define (qq-expand-list x)
  (cond
    [(not (pair? x)) (list 'list (list 'quote x))]
    [(= 'unquote (car x)) (list 'list (cadr x))]
    [(= 'unquote-splicing (car x)) (cadr x)]
    [(= 'quasiquote (car x))
     (qq-expand-list (qq-expand (cadr x)))]
    [else (list 'list
             (list 'append
               (qq-expand-list (car x))
               (qq-expand (cdr x))))]))

(define (qq-expand x)
  (cond
    [(not (pair? x)) (list 'quote x)]
    [(= 'unquote (car x)) (cadr x)]
    [(= 'unquote-splicing (car x)) (error "quasiquote:" x)]
    [(= 'quasiquote (car x))
     (qq-expand (qq-expand (cadr x)))]
    [else (list 'append
             (qq-expand-list (car x))
             (qq-expand (cdr x)))]))

(define quasiquote (macro (x) (qq-expand x)))

(define define-macro
  (macro (args . body)
    (cond
      [(not (symbol? (car args)))
        (error "Name not provided." (car args))]
      [(null? body)
        (error "Body not provided." body)]
      [else
        `(define ,(car args)
          (macro ,(cdr args) ,@body))])))

(define (validate-binding binding)
  (cond
    [(not (pair? binding))
     (error "Binding must be pair." binding)]
    [(not (= (length binding) 2))
     (error "Binding must contain two elements." binding)]
    [(not (symbol? (car binding)))
     (error "First element of binding must be a symbol." (car binding))]))

(define-macro (let bindings . body)
              (for-each validate-binding bindings)
              `(apply
                    (lambda ,(map car bindings) ,@body)
                    (list ,@(map cadr bindings))))
