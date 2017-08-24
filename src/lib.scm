(define (gen-case c)
  (if (eq? (car c) 'else)
    c
    `[(eq? (car __r) ,(list 'quote (car c)))
      (apply
        (lambda ,(cadr c) ,(caddr c))
        (cdr __r))]))

(define (gen-record-case expr cases)
  `(let ([__r ,expr])
     (cond ,@(map gen-case cases))))

(define-macro (record-case expr . cases)
  (gen-record-case expr cases))

(define (gen-recur name bindings body)
  `(apply
     (lambda ()
       (define (,name ,@(map car bindings)) ,@body)
       (apply ,name (list ,@(map cadr bindings))))))

(define-macro (recur name bindings . body)
  (gen-recur name bindings body))

(define (gen-record val vars body)
  `(apply (lambda ,vars ,@body) ,val))

(define-macro (record val vars . body)
  (gen-record val vars body))
