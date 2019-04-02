#!/bin/sh
grep -lr --exclude-dir=".git" -e "MAT_NS_BEGIN" . | xargs sed -i '' -e 's/MAT_NS_BEGIN/ARIASDK_NS_BEGIN/g'
grep -lr --exclude-dir=".git" -e "MAT_NS_END" . | xargs sed -i '' -e 's/MAT_NS_END/ARIASDK_NS_END/g'
