(in-package :asdf-user)

(defsystem "leap"
    :description "Amber related code"
    :version "0.0.1"
    :author "Christian Schafmeister <chris.schaf@verizon.net>"
    :licence "LGPL-3.0"
    :depends-on ()
    :serial t
    :components
    ((:file "packages")
     (:file "core")
     (:file "off")
;;     (:file "commands")
     (:file "leap")
     (:file "pdb")
     ))