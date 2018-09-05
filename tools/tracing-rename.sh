#!/bin/sh

find . -name "*.cpp" -exec sed -i 's/ARIASDK_LOG_DETAIL/LOG_TRACE/g' {} +
find . -name "*.hpp" -exec sed -i 's/ARIASDK_LOG_DETAIL/LOG_TRACE/g' {} +

find . -name "*.cpp" -exec sed -i 's/ARIASDK_LOG_INFO/LOG_INFO/g' {} +
find . -name "*.hpp" -exec sed -i 's/ARIASDK_LOG_INFO/LOG_INFO/g' {} +

find . -name "*.cpp" -exec sed -i 's/ARIASDK_LOG_WARNING/LOG_WARN/g' {} +
find . -name "*.hpp" -exec sed -i 's/ARIASDK_LOG_WARNING/LOG_WARN/g' {} +

find . -name "*.cpp" -exec sed -i 's/ARIASDK_LOG_ERROR/LOG_ERROR/g' {} +
find . -name "*.hpp" -exec sed -i 's/ARIASDK_LOG_ERROR/LOG_ERROR/g' {} +
