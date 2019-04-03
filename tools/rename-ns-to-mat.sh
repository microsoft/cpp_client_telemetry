#!/bin/sh
grep -lr --exclude-dir=".git" -e "ARIASDK_NS_BEGIN" . | xargs sed -i '' -e 's/ARIASDK_NS_BEGIN/MAT_NS_BEGIN/g'
grep -lr --exclude-dir=".git" -e "ARIASDK_NS_END" . | xargs sed -i '' -e 's/ARIASDK_NS_END/MAT_NS_END/g'
