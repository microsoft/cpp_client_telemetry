APP_ALLOW_MISSING_DEPS  := true
#APP_ABI				:= all
APP_ABI					:= x86_64
APP_STL					:= c++_static
# c++_static
APP_OPTIM				:= release
APP_PLATFORM			:= android-29

APP_LDFLAGS				+= -L$(ANDROID_SYSTEM_LIBS)
$(info APP_LDFLAGS       =$(APP_LDFLAGS))

APP_CPPFLAGS			:= -fexceptions
#APP_CPPFLAGS           += -frtti
APP_CPPFLAGS			+= -fno-rtti 
APP_CPPFLAGS			+= -DANDROID

# Remove this if building with platform C++ library
APP_CPPFLAGS        	+= -DANDROID_NDK_BUILD

APP_CPPFLAGS			+= -Wl,-rpath=$(ANDROID_SYSTEM_LIBS)
$(info APP_CPPFLAGS      =$(APP_CPPFLAGS))
