-- generic/all-purpose pipe color definition file. (hack)
-- run this as "program_name | cw-pipe" in your shell. 
command = "cat"
match("bright", "%d")
match("punctuation", "%.")
match("punctuation", "\"")
match("punctuation", "'")
match("punctuation", "`")
match("bright", ":")
match("bright", "%*")
match("bright", "%-")
match("bright", "\\")
match("bright", "/")
match("bright", "%[")
match("bright", "%]")
match("bright", "<")
match("bright", ">")
match("bright", "%(")
match("bright", "%)")
