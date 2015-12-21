;;; atlas.el --- specialized comint.el for ATLAS interpreters

;; Copyright (C) 1998, 1999, 2000, 2001  Free Software Foundation, Inc.

;; Author: Richard Luo <lc@cs.ucla.edu>
;; Maintainer: Richard Luo <lc@cs.ucla.edu>
;; Version: 1.0
;; Keywords: comm languages processes
;; URL: http://wis.cs.ucla.edu/atlas

;; This file is part of GNU Emacs.

;; GNU Emacs is free software; you can redistribute it and/or modify
;; it under the terms of the GNU General Public License as published by
;; the Free Software Foundation; either version 2, or (at your option)
;; any later version.

;; GNU Emacs is distributed in the hope that it will be useful,
;; but WITHOUT ANY WARRANTY; without even the implied warranty of
;; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
;; GNU General Public License for more details.

;; You should have received a copy of the GNU General Public License
;; along with GNU Emacs; see the file COPYING.  If not, write to the
;; Free Software Foundation, Inc., 59 Temple Place - Suite 330,
;; Boston, MA 02111-1307, USA.

;;; Commentary:

;; Modified from sql.el:
;; Author: Alex Schroeder <alex@gnu.org>
;; Maintainer: Alex Schroeder <alex@gnu.org>
;; Version: 1.6.5
;; Keywords: comm languages processes
;; URL: http://www.emacswiki.org/cgi-bin/wiki.pl?SqlMode




;;; Code:

(require 'comint)
;; Need the following to allow GNU Emacs 19 to compile the file.
(require 'regexp-opt)
(require 'custom)

;;; Allow customization

(defgroup ATLAS nil
  "Running a ATLAS interpreter from within Emacs buffers"
  :version "20.4"
  :group 'processes)

;; These three variables will be used as defaults, if set.

(defcustom atlas-user ""
  "*Default username."
  :type 'string
  :group 'ATLAS)

(defcustom atlas-password ""
  "*Default password.

Storing your password in a textfile such as ~/.emacs could be dangerous.
Customizing your password will store it in your ~/.emacs file."
  :type 'string
  :group 'ATLAS)

(defcustom atlas-database ""
  "*Default database."
  :type 'string
  :group 'ATLAS)

(defcustom atlas-server ""
  "*Default server or host."
  :type 'string
  :group 'ATLAS)

;; misc customization of atlas.el behaviour

(defcustom atlas-electric-stuff nil
  "Treat some input as electric.
If set to the symbol `semicolon', then hitting `;' will send current
input in the ATLASi buffer to the process.
If set to the symbol `go', then hitting `go' on a line by itself will
send current input in the ATLASi buffer to the process.
If set to nil, then you must use \\[comint-send-input] in order to send
current input in the ATLASi buffer to the process."
  :type '(choice (const :tag "Nothing" nil)
		 (const :tag "The semikolon `;'" semicolon)
		 (const :tag "The string `go' by itself" go))
  :version "20.8"
  :group 'ATLAS)

(defcustom atlas-pop-to-buffer-after-send-region nil
  "*If t, pop to the buffer ATLAS statements are sent to.

After a call to `atlas-send-region' or `atlas-send-buffer',
the window is split and the ATLASi buffer is shown.  If this
variable is not nil, that buffer's window will be selected
by calling `pop-to-buffer'.  If this variable is nil, that
buffer is shown using `display-buffer'."
  :type 'boolean
  :group 'ATLAS)

;; imenu support for atlas-mode.

(defvar atlas-imenu-generic-expression
  '(("Tables" "^\\s-*create\\s-+table\\s-+\\(\\w+\\)" 1)
    ("Indexes" "^\\s-*create\\s-+index\\s-+\\(\\w+\\)" 1))
  "Define interesting points in the ATLAS buffer for `imenu'.

This is used to set `imenu-generic-expression' when ATLAS mode is
entered.  Subsequent changes to atlas-imenu-generic-expression will not
affect existing ATLAS buffers because imenu-generic-expression is a
local variable.")

;; history file

(defcustom atlas-input-ring-file-name nil
  "*If non-nil, name of the file to read/write input history.

You have to set this variable if you want the history of your commands
saved from one Emacs session to the next.  If this variable is set,
exiting the ATLAS interpreter in an ATLASi buffer will write the input
history to the specified file.  Starting a new process in a ATLASi buffer
will read the input history from the specified file.

This is used to initialize `comint-input-ring-file-name'.

Note that the size of the input history is determined by the variable
`comint-input-ring-size'."
  :type '(choice (const :tag "none" nil)
		 (file))
  :group 'ATLAS)

(defcustom atlas-input-ring-separator "\n--\n"
  "*Separator between commands in the history file.

If set to \"\\n\", each line in the history file will be interpreted as
one command.  Multi-line commands are split into several commands when
the input ring is initialized from a history file.

This variable used to initialize `comint-input-ring-separator'.
`comint-input-ring-separator' is part of Emacs 21; if your Emacs
does not have it, setting `atlas-input-ring-separator' will have no
effect.  In that case multiline commands will be split into several
commands when the input history is read, as if you had set
`atlas-input-ring-separator' to \"\\n\"."
  :type 'string
  :group 'ATLAS)

;; The usual hooks

(defcustom atlas-interactive-mode-hook '()
  "*Hook for customizing `atlas-interactive-mode'."
  :type 'hook
  :group 'ATLAS)

(defcustom atlas-mode-hook '()
  "*Hook for customizing `atlas-mode'."
  :type 'hook
  :group 'ATLAS)

(defcustom atlas-set-atlasi-hook '()
  "*Hook for reacting to changes of `atlas-buffer'.

This is called by `atlas-set-atlasi-buffer' when the value of `atlas-buffer'
is changed."
  :type 'hook
  :group 'ATLAS)

;; Customization for Oracle

(defcustom atlas-oracle-program "atlasplus"
  "*Command to start atlasplus by Oracle.

Starts `atlas-interactive-mode' after doing some setup.

Under NT, \"atlasplus\" usually starts the atlasplus \"GUI\".  In order to
start the atlasplus console, use \"plus33\" or something similar.  You
will find the file in your Orant\\bin directory.

The program can also specify a TCP connection.  See `make-comint'."
  :type 'file
  :group 'ATLAS)

(defcustom atlas-oracle-options nil
  "*List of additional options for `atlas-oracle-program'."
  :type '(repeat string)
  :version "20.8"
  :group 'ATLAS)

;; Customization for MyAtlas

(defcustom atlas-myatlas-program "myatlas"
  "*Command to start myatlas by TcX.

Starts `atlas-interactive-mode' after doing some setup.

The program can also specify a TCP connection.  See `make-comint'."
  :type 'file
  :group 'ATLAS)

(defcustom atlas-myatlas-options nil
  "*List of additional options for `atlas-myatlas-program'.
The following list of options is reported to make things work
on Windows: \"-C\" \"-t\" \"-f\" \"-n\"."
  :type '(repeat string)
  :version "20.8"
  :group 'ATLAS)

;; Customization for Solid

(defcustom atlas-solid-program "solatlas"
  "*Command to start SOLID ATLAS Editor.

Starts `atlas-interactive-mode' after doing some setup.

The program can also specify a TCP connection.  See `make-comint'."
  :type 'file
  :group 'ATLAS)

;; Customization for SyBase

(defcustom atlas-sybase-program "iatlas"
  "*Command to start iatlas by SyBase.

Starts `atlas-interactive-mode' after doing some setup.

The program can also specify a TCP connection.  See `make-comint'."
  :type 'file
  :group 'ATLAS)

(defcustom atlas-sybase-options nil
  "*List of additional options for `atlas-sybase-program'.
Some versions of iatlas might require the -n option in order to work."
  :type '(repeat string)
  :version "20.8"
  :group 'ATLAS)

;; Customization for Informix

(defcustom atlas-informix-program "dbaccess"
  "*Command to start dbaccess by Informix.

Starts `atlas-interactive-mode' after doing some setup.

The program can also specify a TCP connection.  See `make-comint'."
  :type 'file
  :group 'ATLAS)

;; Customization for Ingres

(defcustom atlas-ingres-program "atlas"
  "*Command to start atlas by Ingres.

Starts `atlas-interactive-mode' after doing some setup.

The program can also specify a TCP connection.  See `make-comint'."
  :type 'file
  :group 'ATLAS)

;; Customization for Microsoft

(defcustom atlas-ms-program "iatlas"
  "*Command to start iatlas by Microsoft.

Starts `atlas-interactive-mode' after doing some setup.

The program can also specify a TCP connection.  See `make-comint'."
  :type 'file
  :group 'ATLAS)

;; Customization for Postgres

(defcustom atlas-postgres-program "patlas"
  "Command to start patlas by Postgres.

Starts `atlas-interactive-mode' after doing some setup.

The program can also specify a TCP connection.  See `make-comint'."
  :type 'file
  :group 'ATLAS)

(defcustom atlas-postgres-options '("-P" "pager=off")
  "*List of additional options for `atlas-postgres-program'.
The default setting includes the -P option which breaks older versions
of the patlas client (such as version 6.5.3).  The -P option is equivalent
to the --pset option.  If you want the patlas to prompt you for a user
name, add the string \"-u\" to the list of options.  If you want to
provide a user name on the command line (newer versions such as 7.1),
add your name with a \"-U\" prefix (such as \"-Umark\") to the list."
  :type '(repeat string)
  :version "20.8"
  :group 'ATLAS)

;; Customization for Interbase

(defcustom atlas-interbase-program "iatlas"
  "*Command to start iatlas by Interbase.

Starts `atlas-interactive-mode' after doing some setup.

The program can also specify a TCP connection.  See `make-comint'."
  :type 'file
  :group 'ATLAS)

(defcustom atlas-interbase-options nil
  "*List of additional options for `atlas-interbase-program'."
  :type '(repeat string)
  :version "20.8"
  :group 'ATLAS)

;; Customization for DB2

(defcustom atlas-db2-program "db2"
  "*Command to start db2 by IBM.

Starts `atlas-interactive-mode' after doing some setup.

The program can also specify a TCP connection.  See `make-comint'."
  :type 'file
  :group 'ATLAS)

(defcustom atlas-db2-options nil
  "*List of additional options for `atlas-db2-program'."
  :type '(repeat string)
  :version "20.8"
  :group 'ATLAS)



;;; Variables which do not need customization

(defvar atlas-user-history nil
  "History of usernames used.")

(defvar atlas-database-history nil
  "History of databases used.")

(defvar atlas-server-history nil
  "History of servers used.")

;; Passwords are not kept in a history.

(defvar atlas-buffer nil
  "Current ATLASi buffer.

The global value of atlas-buffer is the name of the latest ATLASi buffer
created.  Any ATLAS buffer created will make a local copy of this value.
See `atlas-interactive-mode' for more on multiple sessions.  If you want
to change the ATLASi buffer a ATLAS mode sends its ATLAS strings to, change
the local value of `atlas-buffer' using \\[atlas-set-atlasi-buffer].")

(defvar atlas-prompt-regexp nil
  "Prompt used to initialize `comint-prompt-regexp'.

You can change `comint-prompt-regexp' on `atlas-interactive-mode-hook'.")

(defvar atlas-prompt-length 0
  "Prompt used to set `left-margin' in `atlas-interactive-mode'.

You can change it on `atlas-interactive-mode-hook'.")

(defvar atlas-alternate-buffer-name nil
  "Buffer-local string used to possibly rename the ATLASi buffer.

Used by `atlas-rename-buffer'.")

;; Keymap for atlas-interactive-mode.

(defvar atlas-interactive-mode-map 
  (let ((map (make-sparse-keymap)))
    (if (functionp 'set-keymap-parent)
	(set-keymap-parent map comint-mode-map); Emacs
      (set-keymap-parents map (list comint-mode-map))); XEmacs
    (if (functionp 'set-keymap-name)
	(set-keymap-name map 'atlas-interactive-mode-map)); XEmacs
    (define-key map (kbd "C-j") 'atlas-accumulate-and-indent)
    (define-key map (kbd "C-c C-w") 'atlas-copy-column)
    (define-key map (kbd "O") 'atlas-magic-go)
    (define-key map (kbd "o") 'atlas-magic-go)
    (define-key map (kbd ";") 'atlas-magic-semicolon)
    map)
  "Mode map used for `atlas-interactive-mode'.
Based on `comint-mode-map'.")

;; Keymap for atlas-mode.

(defvar atlas-mode-map
  (let ((map (make-sparse-keymap)))
    (define-key map (kbd "C-c C-c") 'atlas-compile)
    (define-key map (kbd "C-c C-r") 'atlas-run)
    map)
  "Mode map used for `atlas-mode'.")

;; easy menu for atlas-mode.

(easy-menu-define
 atlas-mode-menu atlas-mode-map
 "Menu for `atlas-mode'."
 '("ATLAS"
   ["Compile" atlas-compile]
   ["Run" atlas-run]

))
(defun atlas-run ()
  "Run ATLaS Program (with auto-compilation)"
  (interactive)
  (let* ((class (if (buffer-file-name)
		     (file-name-sans-extension(buffer-file-name))))
	 (buf (get-buffer-create class)))
    (shell-command  class)
    ))


(defun atlas-compile ()
  "Compile ATLaS source file"
  (interactive)
  (let* ((t1 (current-time-string))
	 (class (if (buffer-file-name)
		    (file-name-sans-extension(buffer-file-name))))
	 (oldBuf (get-buffer-create class))
	 (bufKilled (if (buffer-live-p oldBuf) 
			(kill-buffer oldBuf)))  ;this statement doesn't have any effect.  I suspect there is a conflict with shell-commend.  To distinguish with last compilation result, I add a time-string t1 above.
	 (buf (generate-new-buffer class)))
    (save-buffer)
    (insert-string "Compiling..." class);doesn't work
    (display-buffer class);doesn't work
    (shell-command (concat "del " class))
    (shell-command (concat "c:/atlas/axl/adlc.exe " buffer-file-name ) buf)
    (if (buffer-live-p buf) (display-buffer class)
    (let (buf1 (get-buffer-create class))
      (shell-command (concat "c:/mingw/bin/g++ -o" class " "class ".cc -Ic:/atlas/include c:/atlas/libraries/libadl.a c:/atlas/libraries/libdb.a c:/atlas/libraries/libimdb.a c:/atlas/libraries/librtree.a") buf1)
      (if (buffer-live-p buf1) 
	  (display-buffer class)
	(let* (
	       (buf (get-buffer-create class)))
	  
	  (insert-string (concat t1 "  Compilation Successful!") class)
	  (display-buffer class)))
      )
    
    )))

;; easy menu for atlas-interactive-mode.

(easy-menu-define
 atlas-interactive-mode-menu atlas-interactive-mode-map
 "Menu for `atlas-interactive-mode'."
 '("ATLAS"
   ["Rename Buffer" atlas-rename-buffer t]))

;; Abbreviations -- if you want more of them, define them in your
;; ~/.emacs file.  Abbrevs have to be enabled in your ~/.emacs, too.

(defvar atlas-mode-abbrev-table nil
  "Abbrev table used in `atlas-mode' and `atlas-interactive-mode'.")
(if atlas-mode-abbrev-table
    ()
  (let ((wrapper))
    (define-abbrev-table 'atlas-mode-abbrev-table ())
    (define-abbrev atlas-mode-abbrev-table  "ins" "insert" nil)
    (define-abbrev atlas-mode-abbrev-table  "upd" "update" nil)
    (define-abbrev atlas-mode-abbrev-table  "del" "delete" nil)
    (define-abbrev atlas-mode-abbrev-table  "sel" "select" nil)))

;; Syntax Table

(defvar atlas-mode-syntax-table
  (let ((table (make-syntax-table)))
    ;; C-style comments /**/ (see elisp manual "Syntax Flags"))
    (modify-syntax-entry ?/ ". 14" table)
    (modify-syntax-entry ?* ". 23" table)
    ;; double-dash starts comment
    (if (string-match "XEmacs\\|Lucid" emacs-version)
	(modify-syntax-entry ?- ". 56" table)
      (modify-syntax-entry ?- ". 12b" table))
    ;; newline and formfeed end coments
    (modify-syntax-entry ?\n "> b" table)
    (modify-syntax-entry ?\f "> b" table)
    ;; single quotes (') quotes delimit strings
    (modify-syntax-entry ?' "\"" table)
    ;; backslash is no escape character
    (modify-syntax-entry ?\\ "." table)
    table)
  "Syntax table used in `atlas-mode' and `atlas-interactive-mode'.")

;; Font lock support

(defvar atlas-mode-ansi-font-lock-keywords nil
  "ANSI ATLAS keywords used by font-lock.

This variable is used by `atlas-mode' and `atlas-interactive-mode'.  The
regular expressions are created during compilation by calling the
function `regexp-opt'.  Therefore, take a look at the source before
you define your own atlas-mode-ansi-font-lock-keywords.  You may want to
add functions and PL/ATLAS keywords.")
(if atlas-mode-ansi-font-lock-keywords
    ()
  (let ((ansi-keywords (eval-when-compile
			 (concat "\\b"
				 (regexp-opt '(
"authorization" "avg" "begin" "close" "cobol" "commit"
"continue" "count" "declare" "double" "end" "escape"
"exec" "fetch" "foreign" "fortran" "found" "go" "goto" "indicator"
"key" "language" "max" "min" "module" "numeric" "open" "pascal" "pli"
"precision" "primary" "procedure" "references" "rollback"
"schema" "section" "some" "sqlcode" "sqlerror" "sum" "work") t) "\\b")))
	(ansi-reserved-words (eval-when-compile
			       (concat "\\b"
				       (regexp-opt '(
"aggregate" "all" "and" "as" "asc" "btree" "by" "case" "create" "delete" "desc" "distinct" "double" "drop" "else" "end" "except" "exists" "external"  "from" "function" "group" "having" "in" "initialize" "insert" "intersect" "into" "iterate" "load" "local" "memory" "not" "null" "oid" "or" "order" "ordered" "primary" "real" "ref" "return" "rtree" "select" "set" "sorted" "source" "system" "state" "table" "terminate" "type" "union" "unique" "update" "values" "when" "where"
) t) "\\b")))
	(ansi-types (eval-when-compile
		      (concat "\\b"
			      (regexp-opt '(
;; ANSI Keywords that look like types
"character" "cursor" "dec" "int" "real" "float"
;; ANSI Reserved Word that look like types
"char" "integer" "smallint" ) t) "\\b"))))
    (setq atlas-mode-ansi-font-lock-keywords
	  (list (cons ansi-keywords 'font-lock-function-name-face)
		(cons ansi-reserved-words 'font-lock-keyword-face)
		(cons ansi-types 'font-lock-type-face)))))

(defvar atlas-mode-oracle-font-lock-keywords nil
  "Oracle ATLAS keywords used by font-lock.

This variable is used by `atlas-mode' and `atlas-interactive-mode'.  The
regular expressions are created during compilation by calling the
function `regexp-opt'.  Therefore, take a look at the source before
you define your own atlas-mode-oracle-font-lock-keywords.  You may want
to add functions and PL/ATLAS keywords.")
(if atlas-mode-oracle-font-lock-keywords
    ()
  (let ((oracle-keywords (eval-when-compile
			   (concat "\\b"
				   (regexp-opt '(
"admin" "after" "allocate" "analyze" "archive" "archivelog" "backup"
"become" "before" "block" "body" "cache" "cancel" "cascade" "change"
"checkpoint" "compile" "constraint" "constraints" "contents"
"controlfile" "cycle" "database" "datafile" "dba" "disable" "dismount"
"dump" "each" "else" "elsif" "enable" "events" "except" "exceptions"
"execute" "exit" "explain" "extent" "externally" "false" "flush" "force"
"freelist" "freelists" "function" "groups" "if" "including" "initrans"
"instance" "layer" "link" "lists" "logfile" "loop" "manage" "manual"
"maxdatafiles" "maxinistances" "maxlogfiles" "maxloghistory"
"maxlogmembers" "maxtrans" "maxvalue" "minextents" "minvalue" "mount"
"new" "next" "noarchivelog" "nocache" "nocycle" "nomaxvalue"
"nominvalue" "none" "noorder" "noresetlogs" "normal" "nosort" "off"
"old" "only" "optimal" "others" "out" "own" "package" "parallel"
"pctincrease" "pctused" "plan" "pragma" "private" "profile" "quota"
"raise" "read" "recover" "referencing" "resetlogs" "restrict_references"
"restricted" "return" "returning" "reuse" "rnds" "rnps" "role" "roles"
"savepoint" "scn" "segment" "sequence" "shared" "snapshot" "sort"
"statement_id" "statistics" "stop" "storage" "subtype" "switch" "system"
"tables" "tablespace" "temporary" "thread" "time" "tracing"
"transaction" "triggers" "true" "truncate" "type" "under" "unlimited"
"until" "use" "using" "when" "while" "wnds" "wnps" "write") t) "\\b")))
	(oracle-warning-words (eval-when-compile
				 (concat "\\b"
					 (regexp-opt '(
"cursor_already_open" "dup_val_on_index" "exception" "invalid_cursor"
"invalid_number" "login_denied" "no_data_found" "not_logged_on"
"notfound" "others" "pragma" "program_error" "storage_error"
"timeout_on_resource" "too_many_rows" "transaction_backed_out"
"value_error" "zero_divide") t) "\\b")))
	(oracle-reserved-words (eval-when-compile
				 (concat "\\b"
					 (regexp-opt '(
"access" "add" "alter" "audit" "cluster" "column" "comment" "compress"
"connect" "drop" "else" "exclusive" "file" "grant"
"identified" "immediate" "increment" "index" "initial" "intersect"
"level" "lock" "long" "maxextents" "minus" "mode" "modify" "noaudit"
"nocompress" "nowait" "number" "offline" "online" "pctfree" "prior"
"raw" "rename" "resource" "revoke" "row" "rowlabel" "rownum"
"rows" "session" "share" "size" "start" "successful" "synonym" "sysdate"
"then" "trigger" "uid" "validate" "whenever") t) "\\b")))
	(oracle-types (eval-when-compile
			(concat "\\b"
				(regexp-opt '(
;; Oracle Keywords that look like types
;; Oracle Reserved Words that look like types
"binary_integer" "blob" "boolean" "constant" "date" "decimal" "rowid"
"varchar" "varchar2") t) "\\b")))
	(oracle-builtin-functions (eval-when-compile
			(concat "\\b"
				(regexp-opt '(
;; Misc Oracle builtin functions
"abs" "add_months" "ascii" "avg" "ceil" "chartorowid" "chr" "concat"
"convert" "cos" "cosh" "count" "currval" "decode" "dump" "exp" "floor"
"glb" "greatest" "greatest_lb" "hextoraw" "initcap" "instr" "instrb"
"last_day" "least" "least_ub" "length" "lengthb" "ln" "log" "lower"
"lpad" "ltrim" "lub" "max" "min" "mod" "months_between" "new_time"
"next_day" "nextval" "nls_initcap" "nls_lower" "nls_upper" "nlssort"
"nvl" "power" "rawtohex" "replace" "round" "rowidtochar" "rpad"
"rtrim" "sign" "sin" "sinh" "soundex" "atlascode" "atlaserrm" "sqrt"
"stddev" "sum" "substr" "substrb" "tan" "tanh" "to_char"
"to_date" "to_label" "to_multi_byte" "to_number" "to_single_byte"
"translate" "trim" "trunc" "uid" "upper" "userenv" "variance" "vsize") t) "\\b"))))
    (setq atlas-mode-oracle-font-lock-keywords
	  (append atlas-mode-ansi-font-lock-keywords
		  (list (cons oracle-keywords 'font-lock-function-name-face)
			(cons oracle-warning-words 'font-lock-warning-face)
			(cons oracle-reserved-words 'font-lock-keyword-face)
			;; XEmacs doesn't have font-lock-builtin-face
			(if (string-match "XEmacs\\|Lucid" emacs-version)
			    (cons oracle-builtin-functions 'font-lock-preprocessor-face)
			  ;; GNU Emacs 19 doesn't have it either
			  (if (string-match "GNU Emacs 19" emacs-version)
			      (cons oracle-builtin-functions 'font-lock-function-name-face)
			    ;; Emacs
			    (cons oracle-builtin-functions 'font-lock-builtin-face)))
			(cons oracle-types 'font-lock-type-face))))))

(defvar atlas-mode-postgres-font-lock-keywords nil
  "Postgres ATLAS keywords used by font-lock.

This variable is used by `atlas-mode' and `atlas-interactive-mode'.  The
regular expressions are created during compilation by calling the
function `regexp-opt'.  Therefore, take a look at the source before
you define your own atlas-mode-postgres-font-lock-keywords.")

(if atlas-mode-postgres-font-lock-keywords
    ()
  (let ((postgres-reserved-words (eval-when-compile
				 (concat "\\b"
					 (regexp-opt '(
"language"
) t) "\\b")))
	(postgres-types (eval-when-compile
			  (concat "\\b"
				  (regexp-opt '(
"bool" "box" "circle" "char" "char2" "char4" "char8" "char16" "date"
"float4" "float8" "int2" "int4" "int8" "line" "lseg" "money" "path"
"point" "polygon" "serial" "text" "time" "timespan" "timestamp" "varchar"
) t)"\\b")))
	(postgres-builtin-functions (eval-when-compile
			(concat "\\b"
				(regexp-opt '(
;; Misc Postgres builtin functions
"abstime" "age" "area" "box" "center" "date_part" "date_trunc"
"datetime" "dexp" "diameter" "dpow" "float" "float4" "height"
"initcap" "integer" "isclosed" "isfinite" "isoldpath" "isopen"
"length" "lower" "lpad" "ltrim" "pclose" "point" "points" "popen"
"position" "radius" "reltime" "revertpoly" "rpad" "rtrim" "substr"
"substring" "text" "timespan" "translate" "trim" "upgradepath"
"upgradepoly" "upper" "varchar" "width"
) t) "\\b"))))
    (setq atlas-mode-postgres-font-lock-keywords
	  (append atlas-mode-ansi-font-lock-keywords
		  (list (cons postgres-reserved-words 'font-lock-keyword-face)
			;; XEmacs doesn't have 'font-lock-builtin-face
			(if (string-match "XEmacs\\|Lucid" emacs-version)
			    (cons postgres-builtin-functions 'font-lock-preprocessor-face)
			  ;; Emacs
			  (cons postgres-builtin-functions 'font-lock-builtin-face))
			(cons postgres-types 'font-lock-type-face))))))


(defvar atlas-mode-font-lock-keywords atlas-mode-ansi-font-lock-keywords
  "ATLAS keywords used by font-lock.

This variable defaults to `atlas-mode-ansi-font-lock-keywords'.  This is
used for the default `font-lock-defaults' value in `atlas-mode'.  This
can be changed by some entry functions to provide more hilighting.")



;;; Functions to switch highlighting

(defun atlas-highlight-oracle-keywords ()
  "Highlight Oracle keywords.
Basically, this just sets `font-lock-keywords' appropriately."
  (interactive)
  (setq font-lock-keywords atlas-mode-oracle-font-lock-keywords)
  (font-lock-fontify-buffer))

(defun atlas-highlight-postgres-keywords ()
  "Highlight Postgres keywords.
Basically, this just sets `font-lock-keywords' appropriately."
  (interactive)
  (setq font-lock-keywords atlas-mode-postgres-font-lock-keywords)
  (font-lock-fontify-buffer))

(defun atlas-highlight-ansi-keywords ()
  "Highlight ANSI ATLAS keywords.
Basically, this just sets `font-lock-keywords' appropriately."
  (interactive)
  (setq font-lock-keywords atlas-mode-ansi-font-lock-keywords)
  (font-lock-fontify-buffer))



;;; Compatibility functions

(if (not (fboundp 'comint-line-beginning-position))
    ;; comint-line-beginning-position is defined in Emacs 21
    (defun comint-line-beginning-position ()
      "Returns the buffer position of the beginning of the line, after any prompt.
The prompt is assumed to be any text at the beginning of the line matching
the regular expression `comint-prompt-regexp', a buffer local variable."
      (save-excursion (comint-bol nil) (point))))



;;; Small functions

(defun atlas-magic-go (arg)
  "Insert \"o\" and call `comint-send-input'.
`atlas-electric-stuff' must be the symbol `go'."
  (interactive "P")
  (self-insert-command (prefix-numeric-value arg))
  (if (and (equal atlas-electric-stuff 'go)
	   (save-excursion
	     (comint-bol nil)
	     (looking-at "go\\b")))
      (comint-send-input)))

(defun atlas-magic-semicolon (arg)
  "Insert semicolon and call `comint-send-input'.
`atlas-electric-stuff' must be the symbol `semicolon'."
  (interactive "P")
  (self-insert-command (prefix-numeric-value arg))
  (if (equal atlas-electric-stuff 'semicolon)
       (comint-send-input)))

(defun atlas-accumulate-and-indent ()
  "Continue ATLAS statement on the next line."
  (interactive)
  (if (fboundp 'comint-accumulate) 
      (comint-accumulate)
    (newline))
  (indent-according-to-mode))

;;;###autoload
(defun atlas-help ()
  "Show short help for the ATLAS modes.

Use an entry function to open an interactive ATLAS buffer.  This buffer is
usually named `*ATLAS*'.  The name of the major mode is ATLASi.

Use the following commands to start a specific ATLAS interpreter:

    PostGres: \\[atlas-postgres]
    MyATLAS: \\[atlas-myatlas]

Other non-free ATLAS implementations are also supported:

    Solid: \\[atlas-solid]
    Oracle: \\[atlas-oracle]
    Informix: \\[atlas-informix]
    Sybase: \\[atlas-sybase]
    Ingres: \\[atlas-ingres]
    Microsoft: \\[atlas-ms]
    Interbase: \\[atlas-interbase]

But we urge you to choose a free implementation instead of these.

Once you have the ATLASi buffer, you can enter ATLAS statements in the
buffer.  The output generated is appended to the buffer and a new prompt
is generated.  See the In/Out menu in the ATLASi buffer for some functions
that help you navigate through the buffer, the input history, etc.

If you have a really complex ATLAS statement or if you are writing a
procedure, you can do this in a separate buffer.  Put the new buffer in
`atlas-mode' by calling \\[atlas-mode].  The name of this buffer can be
anything.  The name of the major mode is ATLAS.

In this ATLAS buffer (ATLAS mode), you can send the region or the entire
buffer to the interactive ATLAS buffer (ATLASi mode).  The results are
appended to the ATLASi buffer without disturbing your ATLAS buffer."
  (interactive)
  (describe-function 'atlas-help))

(defun atlas-read-passwd (prompt &optional default)
  "Read a password using PROMPT.
Optional DEFAULT is password to start with.  This function calls
`read-passwd' if it is available.  If not, function
`ange-ftp-read-passwd' is called.  This should always be available,
even in old versions of Emacs."
  (if (fboundp 'read-passwd)
      (read-passwd prompt nil default)
    (unless (fboundp 'ange-ftp-read-passwd)
      (autoload 'ange-ftp-read-passwd "ange-ftp"))
    (ange-ftp-read-passwd prompt default)))

(defun atlas-get-login (&rest what)
  "Get username, password and database from the user.

The variables `atlas-user', `atlas-password', `atlas-server', and
`atlas-database' can be customized.  They are used as the default values.
Usernames, servers and databases are stored in `atlas-user-history',
`atlas-server-history' and `database-history'.  Passwords are not stored
in a history.

Parameter WHAT is a list of the arguments passed to this function.
The function asks for the username if WHAT contains symbol `user', for
the password if it contains symbol `password', for the server if it
contains symbol `server', and for the database if it contains symbol
`database'.

In order to ask the user for username, password and database, call the
function like this: (atlas-get-login 'user 'password 'database)."
  (interactive)
  (if (memq 'user what)
      (setq atlas-user
	    (read-from-minibuffer "User: " atlas-user nil nil
				  atlas-user-history)))
  (if (memq 'password what)
      (setq atlas-password
	    (atlas-read-passwd "Password: " atlas-password)))
  (if (memq 'server what)
      (setq atlas-server
	    (read-from-minibuffer "Server: " atlas-server nil nil
				  atlas-server-history)))
  (if (memq 'database what)
      (setq atlas-database
	    (read-from-minibuffer "Database: " atlas-database nil nil
				  atlas-database-history))))

(defun atlas-find-atlasi-buffer ()
  "Return the current default ATLASi buffer or nil.
In order to qualify, the ATLASi buffer must be alive,
be in `atlas-interactive-mode' and have a process."
  (let ((default-buffer (default-value 'atlas-buffer)))
    (if (and (buffer-live-p default-buffer)
	     (get-buffer-process default-buffer))
	default-buffer
      (save-excursion
	(let ((buflist (buffer-list))
	      (found))
	  (while (not (or (null buflist)
			  found))
	    (let ((candidate (car buflist)))
	      (set-buffer candidate)
	      (if (and (equal major-mode 'atlas-interactive-mode)
		       (get-buffer-process candidate))
		  (setq found candidate))
	      (setq buflist (cdr buflist))))
	  found)))))

(defun atlas-set-atlasi-buffer-generally ()
  "Set ATLASi buffer for all ATLAS buffers that have none.  
This function checks all ATLAS buffers for their ATLASi buffer.  If their
ATLASi buffer is nonexistent or has no process, it is set to the current
default ATLASi buffer.  The current default ATLASi buffer is determined
using `atlas-find-atlasi-buffer'.  If `atlas-buffer' is set,
`atlas-set-atlasi-hook' is run."
  (interactive)
  (save-excursion
    (let ((buflist (buffer-list))
	  (default-atlasi-buffer (atlas-find-atlasi-buffer)))
      (setq-default atlas-buffer default-atlasi-buffer)
      (while (not (null buflist))
	(let ((candidate (car buflist)))
	  (set-buffer candidate)
	  (if (and (equal major-mode 'atlas-mode)
		   (not (buffer-live-p atlas-buffer)))
	      (progn
		(setq atlas-buffer default-atlasi-buffer)
		(run-hooks 'atlas-set-atlasi-hook))))
	(setq buflist (cdr buflist))))))

(defun atlas-set-atlasi-buffer ()
  "Set the ATLASi buffer ATLAS strings are sent to.

Call this function in a ATLAS buffer in order to set the ATLASi buffer ATLAS
strings are sent to.  Calling this function sets `atlas-buffer' and runs
`atlas-set-atlasi-hook'.

If you call it from a ATLAS buffer, this sets the local copy of
`atlas-buffer'.

If you call it from anywhere else, it sets the global copy of
`atlas-buffer'."
  (interactive)
  (let ((default-buffer (atlas-find-atlasi-buffer)))
    (if (null default-buffer)
	(error "There is no suitable ATLASi buffer"))
    (let ((new-buffer
	   (get-buffer
	    (read-buffer "New ATLASi buffer: " default-buffer t))))
      (if (null (get-buffer-process new-buffer))
	  (error "Buffer %s has no process" (buffer-name new-buffer)))
      (if (null (save-excursion
		  (set-buffer new-buffer)
		  (equal major-mode 'atlas-interactive-mode)))
	  (error "Buffer %s is no ATLASi buffer" (buffer-name new-buffer)))
      (if new-buffer
	  (progn
	    (setq atlas-buffer new-buffer)
	    (run-hooks 'atlas-set-atlasi-hook))))))

(defun atlas-show-atlasi-buffer ()
  "Show the name of current ATLASi buffer.

This is the buffer ATLAS strings are sent to.  It is stored in the
variable `atlas-buffer'.  See `atlas-help' on how to create such a buffer."
  (interactive)
  (if (null (buffer-live-p atlas-buffer))
      (message "%s has no ATLASi buffer set." (buffer-name (current-buffer)))
    (if (null (get-buffer-process atlas-buffer))
	(message "Buffer %s has no process." (buffer-name atlas-buffer))
      (message "Current ATLASi buffer is %s." (buffer-name atlas-buffer)))))

(defun atlas-make-alternate-buffer-name ()
  "Return a string that can be used to rename a ATLASi buffer.

This is used to set `atlas-alternate-buffer-name' within
`atlas-interactive-mode'."
  (concat (if (string= "" atlas-user)
	      (if (string= "" (user-login-name))
		  ()
		(concat (user-login-name) "/"))
	    (concat atlas-user "/"))
	  (if (string= "" atlas-database)
	      (if (string= "" atlas-server)
		  (system-name)
		atlas-server)
	    atlas-database)))

(defun atlas-rename-buffer ()
  "Renames a ATLASi buffer."
  (interactive)
  (rename-buffer (format "*ATLAS: %s*" atlas-alternate-buffer-name) t))

(defun atlas-copy-column ()
  "Copy current column to the end of buffer.
Inserts SELECT or commas if appropriate."
  (interactive)
  (let ((column))
    (save-excursion
      (setq column (buffer-substring
		  (progn (forward-char 1) (backward-sexp 1) (point))
		  (progn (forward-sexp 1) (point))))
      (goto-char (point-max))
      (let ((bol (comint-line-beginning-position)))
	(cond
	 ;; if empty command line, insert SELECT
	 ((= bol (point))
	  (insert "SELECT "))
	 ;; else if appending to INTO .* (, SELECT or ORDER BY, insert a comma
	 ((save-excursion
	    (re-search-backward "\\b\\(\\(into\\s-+\\S-+\\s-+(\\)\\|select\\|order by\\) .+"
				bol t))
	  (insert ", "))
	 ;; else insert a space
	 (t
	  (if (eq (preceding-char) ? )
	      nil
	    (insert " ")))))
      ;; in any case, insert the column
      (insert column)
      (message "%s" column))))

;; On NT, ATLAS*Plus for Oracle turns on full buffering for stdout if it
;; is not attached to a character device; therefore placeholder
;; replacement by ATLAS*Plus is fully buffered.  The workaround lets
;; Emacs query for the placeholders.

(defvar atlas-placeholder-history nil
  "History of placeholder values used.")

(defun atlas-query-placeholders-and-send (proc string)
  "Send to PROC input STRING, maybe replacing placeholders.
Placeholders are words starting with and ampersand like &this.
This function is used for `comint-input-sender' if using `atlas-oracle' on NT."
  (while (string-match "&\\(\\sw+\\)" string)
    (setq string (replace-match 
		  (read-from-minibuffer
		   (format "Enter value for %s: " (match-string 1 string))
		   nil nil nil atlas-placeholder-history)
		  t t string)))
  (comint-send-string proc string)
  (comint-send-string proc "\n"))

;; Using DB2 interactively, newlines must be escaped with " \".
;; The space before the backslash is relevant.
(defun atlas-escape-newlines-and-send (proc string)
  "Send to PROC input STRING, escaping newlines if necessary.
Every newline in STRING will be preceded with a space and a backslash."
  (let ((result "") (start 0) mb me)
    (while (string-match "\n" string start)
      (setq mb (match-beginning 0)
	    me (match-end 0))
      (if (and (> mb 1)
	       (string-equal " \\" (substring string (- mb 2) mb)))
	  (setq result (concat result (substring string start me)))
	(setq result (concat result (substring string start mb) " \\\n")))
      (setq start me))
    (setq result (concat result (substring string start)))
    (comint-send-string proc result)
    (comint-send-string proc "\n")))



;;; Sending the region to the ATLASi buffer.

(defun atlas-send-region (start end)
  "Send a region to the ATLAS process."
  (interactive "r")
  (if (buffer-live-p atlas-buffer)
      (save-excursion
	(comint-send-region atlas-buffer start end)
	(if (string-match "\n$" (buffer-substring start end))
	    ()
	  (comint-send-string atlas-buffer "\n"))
	(message "Sent string to buffer %s." (buffer-name atlas-buffer))
	(if atlas-pop-to-buffer-after-send-region
	    (pop-to-buffer atlas-buffer)
	  (display-buffer atlas-buffer)))
    (message "No ATLAS process started.")))

(defun atlas-send-paragraph ()
  "Send the current paragraph to the ATLAS process."
  (interactive)
  (let ((start (save-excursion
		 (backward-paragraph)
		 (point)))
	(end (save-excursion
	       (forward-paragraph)
	       (point))))
    (atlas-send-region start end)))

(defun atlas-send-buffer ()
  "Send the buffer contents to the ATLAS process."
  (interactive)
  (atlas-send-region (point-min) (point-max)))

(defun atlas-toggle-pop-to-buffer-after-send-region (&optional value)
  "Toggle `atlas-pop-to-buffer-after-send-region'.

If given the optional parameter VALUE, sets
atlas-toggle-pop-to-buffer-after-send-region to VALUE."
  (interactive "P")
  (if value
      (setq atlas-pop-to-buffer-after-send-region value)
    (setq atlas-pop-to-buffer-after-send-region
	  (null atlas-pop-to-buffer-after-send-region ))))



;;; ATLAS mode -- uses ATLAS interactive mode

;;;###autoload
(defun atlas-mode ()
  "Major mode to edit ATLAS.

You can send ATLAS statements to the ATLASi buffer using
\\[atlas-send-region].  Such a buffer must exist before you can do this.
See `atlas-help' on how to create ATLASi buffers.

\\{atlas-mode-map}
Customization: Entry to this mode runs the `atlas-mode-hook'.

When you put a buffer in ATLAS mode, the buffer stores the last ATLASi
buffer created as its destination in the variable `atlas-buffer'.  This
will be the buffer \\[atlas-send-region] sends the region to.  If this
ATLASi buffer is killed, \\[atlas-send-region] is no longer able to
determine where the strings should be sent to.  You can set the
value of `atlas-buffer' using \\[atlas-set-atlasi-buffer].

For information on how to create multiple ATLASi buffers, see
`atlas-interactive-mode'.

Note that ATLAS doesn't really have an escape character.  If you want
to use the backslash as an escape character, you must tell Emacs.
Here's how to do that in your ~/.emacs file:

\(add-hook 'atlas-mode-hook
          \(lambda ()
	    \(modify-syntax-entry ?\\\\ \".\" atlas-mode-syntax-table)))"
  (interactive)
  (kill-all-local-variables)
  (setq major-mode 'atlas-mode)
  (setq mode-name "ATLAS")
  (use-local-map atlas-mode-map)
  (if atlas-mode-menu
      (easy-menu-add atlas-mode-menu)); XEmacs
  (set-syntax-table atlas-mode-syntax-table)
  (make-local-variable 'font-lock-defaults)
  ;; Note that making KEYWORDS-ONLY nil will cause havoc if you try
  ;; SELECT 'x' FROM DUAL with ATLAS*Plus, because the title of the column
  ;; will have just one quote.  Therefore syntactic hilighting is
  ;; disabled for interactive buffers.  `_' and `.' are considered part
  ;; of words.
  (setq font-lock-defaults '(atlas-mode-font-lock-keywords
			     nil t ((?_ . "w") (?. . "w"))))
  (make-local-variable 'comment-start)
  (setq comment-start "--")
  ;; Make each buffer in atlas-mode remember the "current" ATLASi buffer.
  (make-local-variable 'atlas-buffer)
  ;; Add imenu support for atlas-mode.  Note that imenu-generic-expression
  ;; is buffer-local, so we don't need a local-variable for it.  ATLAS is
  ;; case-insensitive, that's why we have to set imenu-case-fold-search.
  ;; imenu-syntax-alist makes sure that `_' is considered part of object
  ;; names.
  (setq imenu-generic-expression atlas-imenu-generic-expression
	imenu-case-fold-search t
	imenu-syntax-alist '(("_" . "w")))
  ;; Make `atlas-send-paragraph' work on paragraphs that contain indented
  ;; lines.
  (make-local-variable 'paragraph-separate)
  (make-local-variable 'paragraph-start)
  (setq paragraph-separate "[\f]*$"
	paragraph-start "[\n\f]")
  ;; Abbrevs
  (setq local-abbrev-table atlas-mode-abbrev-table)
  (setq abbrev-all-caps 1)
  ;; Run hook
  (run-hooks 'atlas-mode-hook))



;;; ATLAS interactive mode

(put 'atlas-interactive-mode 'mode-class 'special)

(defun atlas-interactive-mode ()
  "Major mode to use a ATLAS interpreter interactively.

Do not call this function by yourself.  The environment must be
initialized by an entry function specific for the ATLAS interpreter.  See
`atlas-help' for a list of available entry functions.

\\[comint-send-input] after the end of the process' output sends the
text from the end of process to the end of the current line.
\\[comint-send-input] before end of process output copies the current
line minus the prompt to the end of the buffer and sends it.
\\[comint-copy-old-input] just copies the current line.
Use \\[atlas-accumulate-and-indent] to enter multi-line statements.

If you want to make multiple ATLAS buffers, rename the `*ATLAS*' buffer
using \\[rename-buffer] or \\[rename-uniquely] and start a new process.
See `atlas-help' for a list of available entry functions.  The last buffer
created by such an entry function is the current ATLASi buffer.  ATLAS
buffers will send strings to the ATLASi buffer current at the time of
their creation.  See `atlas-mode' for details.

Sample session using two connections:

1. Create first ATLASi buffer by calling an entry function.
2. Rename buffer \"*ATLAS*\" to \"*Connection 1*\".
3. Create a ATLAS buffer \"test1.atlas\".
4. Create second ATLASi buffer by calling an entry function.
5. Rename buffer \"*ATLAS*\" to \"*Connection 2*\".
6. Create a ATLAS buffer \"test2.atlas\".

Now \\[atlas-send-region] in buffer \"test1.atlas\" will send the region to
buffer \"*Connection 1*\", \\[atlas-send-region] in buffer \"test2.atlas\"
will send the region to buffer \"*Connection 2*\".

If you accidentally suspend your process, use \\[comint-continue-subjob]
to continue it.  On some operating systems, this will not work because
the signals are not supported.

\\{atlas-interactive-mode-map}
Customization: Entry to this mode runs the hooks on `comint-mode-hook'
and `atlas-interactive-mode-hook' (in that order).  Before each input, the
hooks on `comint-input-filter-functions' are run.  After each ATLAS
interpreter output, the hooks on `comint-output-filter-functions' are
run.

Variable `atlas-input-ring-file-name' controls the initialisation of the
input ring history.

Variables `comint-output-filter-functions', a hook, and
`comint-scroll-to-bottom-on-input' and
`comint-scroll-to-bottom-on-output' control whether input and output
cause the window to scroll to the end of the buffer.

If you want to make ATLAS buffers limited in length, add the function
`comint-truncate-buffer' to `comint-output-filter-functions'.

Here is an example for your .emacs file.  It keeps the ATLASi buffer a
certain length.

\(add-hook 'atlas-interactive-mode-hook
    \(function (lambda ()
        \(setq comint-output-filter-functions 'comint-truncate-buffer))))

Here is another example.  It will always put point back to the statement
you entered, right above the output it created.

\(setq comint-output-filter-functions
       \(function (lambda (STR) (comint-show-output))))"
  (comint-mode)
  (setq comint-prompt-regexp atlas-prompt-regexp)
  (setq left-margin atlas-prompt-length)
  (setq major-mode 'atlas-interactive-mode)
  (setq mode-name "ATLASi")
  (use-local-map atlas-interactive-mode-map)
  (if atlas-interactive-mode-menu
      (easy-menu-add atlas-interactive-mode-menu)); XEmacs
  (set-syntax-table atlas-mode-syntax-table)
  (make-local-variable 'font-lock-defaults)
  ;; Note that making KEYWORDS-ONLY nil will cause havoc if you try
  ;; SELECT 'x' FROM DUAL with ATLAS*Plus, because the title of the column
  ;; will have just one quote.  Therefore syntactic hilighting is
  ;; disabled for interactive buffers.  `_' and `.' are considered part
  ;; of words.
  (setq font-lock-defaults '(atlas-mode-font-lock-keywords
			     t t ((?_ . "w") (?. . "w"))))
  ;; Enable commenting and uncommenting of the region.
  (make-local-variable 'comment-start)
  (setq comment-start "--")
  ;; Abbreviation table init and case-insensitive.  It is not activatet
  ;; by default.
  (setq local-abbrev-table atlas-mode-abbrev-table)
  (setq abbrev-all-caps 1)
  ;; Exiting the process will call atlas-stop.
  (set-process-sentinel (get-buffer-process atlas-buffer) 'atlas-stop)
  ;; People wanting a different history file for each
  ;; buffer/process/client/whatever can change separator and file-name
  ;; on the atlas-interactive-mode-hook.
  (setq comint-input-ring-separator atlas-input-ring-separator
	comint-input-ring-file-name atlas-input-ring-file-name)
  ;; Create a usefull name for renaming this buffer later.
  (make-local-variable 'atlas-alternate-buffer-name)
  (setq atlas-alternate-buffer-name (atlas-make-alternate-buffer-name))
  ;; User stuff.
  (run-hooks 'atlas-interactive-mode-hook)
  ;; Calling the hook before calling comint-read-input-ring allows users
  ;; to set comint-input-ring-file-name in atlas-interactive-mode-hook.
  (comint-read-input-ring t))

(defun atlas-stop (process event)
  "Called when the ATLAS process is stopped.

Writes the input history to a history file using
`comint-write-input-ring' and inserts a short message in the ATLAS buffer.

This function is a sentinel watching the ATLAS interpreter process.
Sentinels will always get the two parameters PROCESS and EVENT."
  (comint-write-input-ring)
  (if (and (eq (current-buffer) atlas-buffer)
	   (not buffer-read-only))
      (insert (format "\nProcess %s %s\n" process event))
    (message "Process %s %s" process event)))



;;; Entry functions for different ATLAS interpreters.

;;;###autoload
(defun atlas-oracle ()
  "Run atlasplus by Oracle as an inferior process.

If buffer `*ATLAS*' exists but no process is running, make a new process.
If buffer exists and a process is running, just switch to buffer
`*ATLAS*'.

Interpreter used comes from variable `atlas-oracle-program'.  Login uses
the variables `atlas-user', `atlas-password', and `atlas-database' as
defaults, if set.  Additional command line parameters can be stored in
the list `atlas-oracle-options'.

The buffer is put in atlas-interactive-mode, giving commands for sending
input.  See `atlas-interactive-mode'.

To specify a coding system for converting non-ASCII characters
in the input and output to the process, use \\[universal-coding-system-argument]
before \\[atlas-oracle].  You can also specify this with \\[set-buffer-process-coding-system]
in the ATLAS buffer, after you start the process.
The default comes from `process-coding-system-alist' and
`default-process-coding-system'.

\(Type \\[describe-mode] in the ATLAS buffer for a list of commands.)"
  (interactive)
  (if (comint-check-proc "*ATLAS*")
      (pop-to-buffer "*ATLAS*")
    (atlas-get-login 'user 'password 'database)
    (message "Login...")
    ;; Produce user/password@database construct.  Password without user
    ;; is meaningless; database without user/password is meaningless,
    ;; because "@param" will ask atlasplus to interpret the script
    ;; "param".
    (let ((parameter nil))
      (if (not (string= "" atlas-user))
	  (if (not (string= "" atlas-password))
	      (setq parameter (concat atlas-user "/" atlas-password))
	    (setq parameter atlas-user)))
      (if (and parameter (not (string= "" atlas-database)))
	  (setq parameter (concat parameter "@" atlas-database)))
      (if parameter
	  (setq parameter (nconc (list parameter) atlas-oracle-options))
	(setq parameter atlas-oracle-options))
      (if parameter
	  (set-buffer (apply 'make-comint "ATLAS" atlas-oracle-program nil 
			     parameter))
	(set-buffer (make-comint "ATLAS" atlas-oracle-program nil))))
    (setq atlas-prompt-regexp "^ATLAS> ")
    (setq atlas-prompt-length 5)
    (setq atlas-buffer (current-buffer))
    ;; set atlas-mode-font-lock-keywords to something different before
    ;; calling atlas-interactive-mode.
    (setq atlas-mode-font-lock-keywords atlas-mode-oracle-font-lock-keywords)
    (atlas-interactive-mode)
    ;; If running on NT, make sure we do placeholder replacement
    ;; ourselves.  This must come after atlas-interactive-mode because all
    ;; local variables will be killed, there.
    (if (eq window-system 'w32)
	(setq comint-input-sender 'atlas-query-placeholders-and-send))
    (message "Login...done")
    (pop-to-buffer atlas-buffer)))



;;;###autoload
(defun atlas-sybase ()
  "Run iatlas by SyBase as an inferior process.

If buffer `*ATLAS*' exists but no process is running, make a new process.
If buffer exists and a process is running, just switch to buffer
`*ATLAS*'.

Interpreter used comes from variable `atlas-sybase-program'.  Login uses
the variables `atlas-server', `atlas-user', `atlas-password', and
`atlas-database' as defaults, if set.  Additional command line parameters
can be stored in the list `atlas-sybase-options'.

The buffer is put in atlas-interactive-mode, giving commands for sending
input.  See `atlas-interactive-mode'.

To specify a coding system for converting non-ASCII characters
in the input and output to the process, use \\[universal-coding-system-argument]
before \\[atlas-sybase].  You can also specify this with \\[set-buffer-process-coding-system]
in the ATLAS buffer, after you start the process.
The default comes from `process-coding-system-alist' and
`default-process-coding-system'.

\(Type \\[describe-mode] in the ATLAS buffer for a list of commands.)"
  (interactive)
  (if (comint-check-proc "*ATLAS*")
      (pop-to-buffer "*ATLAS*")
    (atlas-get-login 'server 'user 'password 'database)
    (message "Login...")
    ;; Put all parameters to the program (if defined) in a list and call
    ;; make-comint.
    (let ((params atlas-sybase-options))
      (if (not (string= "" atlas-server))
	  (setq params (append (list "-S" atlas-server) params)))
      (if (not (string= "" atlas-database))
	  (setq params (append (list "-D" atlas-database) params)))
      (if (not (string= "" atlas-password))
	  (setq params (append (list "-P" atlas-password) params)))
      (if (not (string= "" atlas-user))
	  (setq params (append (list "-U" atlas-user) params)))
      (set-buffer (apply 'make-comint "ATLAS" atlas-sybase-program
			 nil params)))
    (setq atlas-prompt-regexp "^ATLAS> ")
    (setq atlas-prompt-length 5)
    (setq atlas-buffer (current-buffer))
    (atlas-interactive-mode)
    (message "Login...done")
    (pop-to-buffer atlas-buffer)))



;;;###autoload
(defun atlas-informix ()
  "Run dbaccess by Informix as an inferior process.

If buffer `*ATLAS*' exists but no process is running, make a new process.
If buffer exists and a process is running, just switch to buffer
`*ATLAS*'.

Interpreter used comes from variable `atlas-informix-program'.  Login uses
the variable `atlas-database' as default, if set.

The buffer is put in atlas-interactive-mode, giving commands for sending
input.  See `atlas-interactive-mode'.

To specify a coding system for converting non-ASCII characters
in the input and output to the process, use \\[universal-coding-system-argument]
before \\[atlas-informix].  You can also specify this with \\[set-buffer-process-coding-system]
in the ATLAS buffer, after you start the process.
The default comes from `process-coding-system-alist' and
`default-process-coding-system'.

\(Type \\[describe-mode] in the ATLAS buffer for a list of commands.)"
  (interactive)
  (if (comint-check-proc "*ATLAS*")
      (pop-to-buffer "*ATLAS*")
    (atlas-get-login 'database)
    (message "Login...")
    ;; username and password are ignored.
    (if (string= "" atlas-database)
	(set-buffer (make-comint "ATLAS" atlas-informix-program nil))
      (set-buffer (make-comint "ATLAS" atlas-informix-program nil atlas-database "-")))
    (setq atlas-prompt-regexp "^ATLAS> ")
    (setq atlas-prompt-length 5)
    (setq atlas-buffer (current-buffer))
    (atlas-interactive-mode)
    (message "Login...done")
    (pop-to-buffer atlas-buffer)))



;;;###autoload
(defun atlas-myatlas ()
  "Run myatlas by TcX as an inferior process.

Myatlas versions 3.23 and up are free software.

If buffer `*ATLAS*' exists but no process is running, make a new process.
If buffer exists and a process is running, just switch to buffer
`*ATLAS*'.

Interpreter used comes from variable `atlas-myatlas-program'.  Login uses
the variables `atlas-user', `atlas-password', `atlas-database', and
`atlas-server' as defaults, if set.  Additional command line parameters
can be stored in the list `atlas-myatlas-options'.

The buffer is put in atlas-interactive-mode, giving commands for sending
input.  See `atlas-interactive-mode'.

To specify a coding system for converting non-ASCII characters
in the input and output to the process, use \\[universal-coding-system-argument]
before \\[atlas-myatlas].  You can also specify this with \\[set-buffer-process-coding-system]
in the ATLAS buffer, after you start the process.
The default comes from `process-coding-system-alist' and
`default-process-coding-system'.

\(Type \\[describe-mode] in the ATLAS buffer for a list of commands.)"
  (interactive)
  (if (comint-check-proc "*ATLAS*")
      (pop-to-buffer "*ATLAS*")
    (atlas-get-login 'user 'password 'database 'server)
    (message "Login...")
    ;; Put all parameters to the program (if defined) in a list and call
    ;; make-comint.
    (let ((params))
      (if (not (string= "" atlas-database))
	  (setq params (append (list atlas-database) params)))
      (if (not (string= "" atlas-server))
	  (setq params (append (list (concat "--host=" atlas-server)) params)))
      (if (not (string= "" atlas-password))
	  (setq params (append (list (concat "--password=" atlas-password)) params)))
      (if (not (string= "" atlas-user))
	  (setq params (append (list (concat "--user=" atlas-user)) params)))
      (if (not (null atlas-myatlas-options))
          (setq params (append atlas-myatlas-options params)))
      (set-buffer (apply 'make-comint "ATLAS" atlas-myatlas-program
			 nil params)))
    (setq atlas-prompt-regexp "^myatlas>")
    (setq atlas-prompt-length 6)
    (setq atlas-buffer (current-buffer))
    (atlas-interactive-mode)
    (message "Login...done")
    (pop-to-buffer atlas-buffer)))



;;;###autoload
(defun atlas-solid ()
  "Run solatlas by Solid as an inferior process.

If buffer `*ATLAS*' exists but no process is running, make a new process.
If buffer exists and a process is running, just switch to buffer
`*ATLAS*'.

Interpreter used comes from variable `atlas-solid-program'.  Login uses
the variables `atlas-user', `atlas-password', and `atlas-server' as
defaults, if set.

The buffer is put in atlas-interactive-mode, giving commands for sending
input.  See `atlas-interactive-mode'.

To specify a coding system for converting non-ASCII characters
in the input and output to the process, use \\[universal-coding-system-argument]
before \\[atlas-solid].  You can also specify this with \\[set-buffer-process-coding-system]
in the ATLAS buffer, after you start the process.
The default comes from `process-coding-system-alist' and
`default-process-coding-system'.

\(Type \\[describe-mode] in the ATLAS buffer for a list of commands.)"
  (interactive)
  (if (comint-check-proc "*ATLAS*")
      (pop-to-buffer "*ATLAS*")
    (atlas-get-login 'user 'password 'server)
    (message "Login...")
    ;; Put all parameters to the program (if defined) in a list and call
    ;; make-comint.
    (let ((params))
      ;; It only makes sense if both username and password are there.
      (if (not (or (string= "" atlas-user)
		   (string= "" atlas-password)))
	  (setq params (append (list atlas-user atlas-password) params)))
      (if (not (string= "" atlas-server))
	  (setq params (append (list atlas-server) params)))
      (set-buffer (apply 'make-comint "ATLAS" atlas-solid-program
			 nil params)))
    (setq atlas-prompt-regexp "^")
    (setq atlas-prompt-length 0)
    (setq atlas-buffer (current-buffer))
    (atlas-interactive-mode)
    (message "Login...done")
    (pop-to-buffer atlas-buffer)))



;;;###autoload
(defun atlas-ingres ()
  "Run atlas by Ingres as an inferior process.

If buffer `*ATLAS*' exists but no process is running, make a new process.
If buffer exists and a process is running, just switch to buffer
`*ATLAS*'.

Interpreter used comes from variable `atlas-ingres-program'.  Login uses
the variable `atlas-database' as default, if set.

The buffer is put in atlas-interactive-mode, giving commands for sending
input.  See `atlas-interactive-mode'.

To specify a coding system for converting non-ASCII characters
in the input and output to the process, use \\[universal-coding-system-argument]
before \\[atlas-ingres].  You can also specify this with \\[set-buffer-process-coding-system]
in the ATLAS buffer, after you start the process.
The default comes from `process-coding-system-alist' and
`default-process-coding-system'.

\(Type \\[describe-mode] in the ATLAS buffer for a list of commands.)"
  (interactive)
  (if (comint-check-proc "*ATLAS*")
      (pop-to-buffer "*ATLAS*")
    (atlas-get-login 'database)
    (message "Login...")
    ;; username and password are ignored.
    (if (string= "" atlas-database)
	(set-buffer (make-comint "ATLAS" atlas-ingres-program nil))
      (set-buffer (make-comint "ATLAS" atlas-ingres-program nil atlas-database)))
    (setq atlas-prompt-regexp "^\* ")
    (setq atlas-prompt-length 2)
    (setq atlas-buffer (current-buffer))
    (atlas-interactive-mode)
    (message "Login...done")
    (pop-to-buffer atlas-buffer)))



;;;###autoload
(defun atlas-ms ()
  "Run iatlas by Microsoft as an inferior process.

If buffer `*ATLAS*' exists but no process is running, make a new process.
If buffer exists and a process is running, just switch to buffer
`*ATLAS*'.

Interpreter used comes from variable `atlas-ms-program'.  Login uses the
variables `atlas-user', `atlas-password', `atlas-database', and `atlas-server'
as defaults, if set.

The buffer is put in atlas-interactive-mode, giving commands for sending
input.  See `atlas-interactive-mode'.

To specify a coding system for converting non-ASCII characters
in the input and output to the process, use \\[universal-coding-system-argument]
before \\[atlas-ms].  You can also specify this with \\[set-buffer-process-coding-system]
in the ATLAS buffer, after you start the process.
The default comes from `process-coding-system-alist' and
`default-process-coding-system'.

\(Type \\[describe-mode] in the ATLAS buffer for a list of commands.)"
  (interactive)
  (if (comint-check-proc "*ATLAS*")
      (pop-to-buffer "*ATLAS*")
    (atlas-get-login 'user 'password 'database 'server)
    (message "Login...")
    ;; Put all parameters to the program (if defined) in a list and call
    ;; make-comint.
    (let ((params '("-w 300")))
      (if (not (string= "" atlas-server))
        (setq params (append (list "-S" atlas-server) params)))
      (if (not (string= "" atlas-database))
        (setq params (append (list "-d" atlas-database) params)))
      (if (not (string= "" atlas-user))
	  (setq params (append (list "-U" atlas-user) params)))
      (if (not (string= "" atlas-password))
	  (setq params (append (list "-P" atlas-password) params))
	;; If -P is passed to IATLAS as the last argument without a password,
	;; it's considered null.
	(setq params (append params (list "-P"))))
      (set-buffer (apply 'make-comint "ATLAS" atlas-ms-program
			 nil params)))
    (setq atlas-prompt-regexp "^[0-9]*>")
    (setq atlas-prompt-length 5)
    (setq atlas-buffer (current-buffer))
    (atlas-interactive-mode)
    (message "Login...done")
    (pop-to-buffer atlas-buffer)))



;;;###autoload
(defun atlas-postgres ()
  "Run patlas by Postgres as an inferior process.

If buffer `*ATLAS*' exists but no process is running, make a new process.
If buffer exists and a process is running, just switch to buffer
`*ATLAS*'.

Interpreter used comes from variable `atlas-postgres-program'.  Login uses
the variables `atlas-database' and `atlas-server' as default, if set.
Additional command line parameters can be stored in the list
`atlas-postgres-options'.

The buffer is put in atlas-interactive-mode, giving commands for sending
input.  See `atlas-interactive-mode'.

To specify a coding system for converting non-ASCII characters
in the input and output to the process, use \\[universal-coding-system-argument]
before \\[atlas-postgres].  You can also specify this with \\[set-buffer-process-coding-system]
in the ATLAS buffer, after you start the process.
The default comes from `process-coding-system-alist' and
`default-process-coding-system'.  If your output lines end with ^M,
your might try undecided-dos as a coding system.  If this doesn't help,
Try to set `comint-output-filter-functions' like this:

\(setq comint-output-filter-functions (append comint-output-filter-functions
					     '(comint-strip-ctrl-m)))

\(Type \\[describe-mode] in the ATLAS buffer for a list of commands.)"
  (interactive)
  (if (comint-check-proc "*ATLAS*")
      (pop-to-buffer "*ATLAS*")
    (atlas-get-login 'database 'server)
    (message "Login...")
    ;; username and password are ignored.  Mark Stosberg suggest to add
    ;; the database at the end.  Jason Beegan suggest using --pset and
    ;; pager=off instead of \\o|cat.  The later was the solution by
    ;; Gregor Zych.  Jason's suggestion is the default value for
    ;; atlas-postgres-options.
    (let ((params atlas-postgres-options))
      (if (not (string= "" atlas-database))
	  (setq params (append params (list atlas-database))))
      (if (not (string= "" atlas-server))
	  (setq params (append (list "-h" atlas-server) params)))
      (set-buffer (apply 'make-comint "ATLAS" atlas-postgres-program
			 nil params)))
    (setq atlas-prompt-regexp "^.*> *")
    (setq atlas-prompt-length 5)
    ;; This is a lousy hack to prevent patlas from truncating it's output
    ;; and giving stupid warnings. If s.o. knows a way to prevent patlas
    ;; from acting this way, then I would be very thankful to
    ;; incorporate this (Gregor Zych <zych@pool.informatik.rwth-aachen.de>)
    ;; (comint-send-string "*ATLAS*" "\\o \| cat\n")
    (setq atlas-mode-font-lock-keywords atlas-mode-postgres-font-lock-keywords)
    (setq atlas-buffer (current-buffer))
    (atlas-interactive-mode)
    (message "Login...done")
    (pop-to-buffer atlas-buffer)))



;;;###autoload
(defun atlas-interbase ()
  "Run iatlas by Interbase as an inferior process.

If buffer `*ATLAS*' exists but no process is running, make a new process.
If buffer exists and a process is running, just switch to buffer
`*ATLAS*'.

Interpreter used comes from variable `atlas-interbase-program'.  Login
uses the variables `atlas-user', `atlas-password', and `atlas-database' as
defaults, if set.

The buffer is put in atlas-interactive-mode, giving commands for sending
input.  See `atlas-interactive-mode'.

To specify a coding system for converting non-ASCII characters
in the input and output to the process, use \\[universal-coding-system-argument]
before \\[atlas-interbase].  You can also specify this with \\[set-buffer-process-coding-system]
in the ATLAS buffer, after you start the process.
The default comes from `process-coding-system-alist' and
`default-process-coding-system'.

\(Type \\[describe-mode] in the ATLAS buffer for a list of commands.)"
  (interactive)
  (if (comint-check-proc "*ATLAS*")
      (pop-to-buffer "*ATLAS*")
    (atlas-get-login 'user 'password 'database)
    (message "Login...")
    ;; Put all parameters to the program (if defined) in a list and call
    ;; make-comint.
    (let ((params atlas-interbase-options))
      (if (not (string= "" atlas-user))
	  (setq params (append (list "-u" atlas-user) params)))
      (if (not (string= "" atlas-password))
	  (setq params (append (list "-p" atlas-password) params)))
      (if (not (string= "" atlas-database))
        (setq params (cons atlas-database params))); add to the front!
      (set-buffer (apply 'make-comint "ATLAS" atlas-interbase-program
			 nil params)))
    (setq atlas-prompt-regexp "^ATLAS> ")
    (setq atlas-prompt-length 5)
    (setq atlas-buffer (current-buffer))
    (atlas-interactive-mode)
    (message "Login...done")
    (pop-to-buffer atlas-buffer)))



;;;###autoload
(defun atlas-db2 ()
  "Run db2 by IBM as an inferior process.

If buffer `*ATLAS*' exists but no process is running, make a new process.
If buffer exists and a process is running, just switch to buffer
`*ATLAS*'.

Interpreter used comes from variable `atlas-db2-program'.  There is not
automatic login.

The buffer is put in atlas-interactive-mode, giving commands for sending
input.  See `atlas-interactive-mode'.

If you use \\[atlas-accumulate-and-indent] to send multiline commands to
db2, newlines will be escaped if necessary.  If you don't want that, set
`comint-input-sender' back to `comint-simple-send' by writing an after
advice.  See the elisp manual for more information.

To specify a coding system for converting non-ASCII characters
in the input and output to the process, use \\[universal-coding-system-argument]
before \\[atlas-db2].  You can also specify this with \\[set-buffer-process-coding-system]
in the ATLAS buffer, after you start the process.
The default comes from `process-coding-system-alist' and
`default-process-coding-system'.

\(Type \\[describe-mode] in the ATLAS buffer for a list of commands.)"
  (interactive)
  (if (comint-check-proc "*ATLAS*")
      (pop-to-buffer "*ATLAS*")
    (message "Login...")
    ;; Put all parameters to the program (if defined) in a list and call
    ;; make-comint.
    (set-buffer (apply 'make-comint "ATLAS" atlas-db2-program
		       nil atlas-db2-options))
    (setq atlas-prompt-regexp "^db2 => ")
    (setq atlas-prompt-length 7)
    (setq atlas-buffer (current-buffer))
    (atlas-interactive-mode)
    ;; Escape newlines.  This must come after atlas-interactive-mode
    ;; because all local variables will be killed, there.
    (setq comint-input-sender 'atlas-escape-newlines-and-send)
    (message "Login...done")
    (pop-to-buffer atlas-buffer)))

;;;###autoload
(add-to-list 'auto-mode-alist '("\\.adl$" . atlas-mode))

(provide 'atlas)

;;; atlas.el ends here
