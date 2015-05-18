#!/bin/bash
#MD_PY=markdown_py
# for python 2.7 and above
#MD_PY="python -m markdown"
# for python 2.6
MD_PY="python -m markdown.__main__ -x markdown.extensions.extra -x markdown.extensions.toc "


markdown_1() {
  FILENAME=$1
  CSS=$2
  cat << END
<html>
<head>
<title>$FILENME</title>
<link rel="stylesheet" href="markdown.css"></link>
<meta charset="UTF-8"></meta>
</head>
<body>
END

  $MD_PY -e utf-8 -x extra -x toc $FILENAME

  cat << END
</body>
<html>
END
}

markdown() {
  FILENAME=$1
  CSS="markdown.css"
  HTML=`echo $FILENAME | sed -e 's/\.md$/\.html/'`

  markdown_1 $FILENAME $CSS | sed -e '1,$s/href=\"\(.*\)\.md\"/href="\1.html"/' >$HTML
}

markdown $1
