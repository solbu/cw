if arg["--help"] then
  match("neutral", "%?")
  match("neutral", "%+")
  match("neutral", " - ")
elseif arg["-"] then
  match("highlight", " %+ ")
  match("lowlight", " - ")
end
match("neutral", "%-")
match("neutral", ":")
match("neutral", "%.")
match("neutral", "=")
match("punctuation", "/")
match("punctuation", "%(")
match("punctuation", "%)")
match("punctuation", "<")
match("punctuation", ">")
match("punctuation", "%[")
match("punctuation", "%]")
match("neutral", "tty")
match("neutral", "pts")
match("punctuation", "LOGIN")
match("neutral", "root")
match("bright", "... .. ..:..")
