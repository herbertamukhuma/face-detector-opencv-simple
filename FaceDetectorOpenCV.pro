# allows to add DEPLOYMENTFOLDERS and links to the Felgo library and QtCreator auto-completion
CONFIG += felgo

QT += multimedia-private

PRODUCT_IDENTIFIER = com.bunistack.FaceDetectorOpenCV
PRODUCT_VERSION_NAME = 1.0.0
PRODUCT_VERSION_CODE = 1

qmlFolder.source = qml
#DEPLOYMENTFOLDERS += qmlFolder

assetsFolder.source = assets
#DEPLOYMENTFOLDERS += assetsFolder

# Add more folders to ship with the application here

RESOURCES += resources.qrc

SOURCES += main.cpp \
    cvfilter.cpp \

HEADERS += \
    cvfilter.h

INCLUDEPATH += C:/opencv/build/include

android {
    ANDROID_PACKAGE_SOURCE_DIR = $$PWD/android
    OTHER_FILES += android/AndroidManifest.xml       android/build.gradle

    equals(ANDROID_TARGET_ARCH,x86) {
        LIBS += -LC:/opencv/build/android/arch-x86/lib/x86
    }

    equals(ANDROID_TARGET_ARCH,armeabi-v7a) {

        LIBS += -LC:/opencv/build/android/arch-arm/install/sdk/native/staticlibs/armeabi-v7a \
                -LC:/opencv/build/android/arch-arm/install/sdk/native/3rdparty/libs/armeabi-v7a

        LIBS += -lopencv_objdetect \
                -lopencv_imgproc \
                -lopencv_core \
                -lcpufeatures \
                -lIlmImf \
                -llibjasper \
                -llibjpeg-turbo \
                -llibpng \
                -llibprotobuf \
                -llibtiff \
                -llibwebp \
                -lquirc \
                -ltegra_hal


    }

    equals(ANDROID_TARGET_ARCH, arm64-v8a)  {
        LIBS += -LC:/opencv/build/android/arch-arm64/lib/arm64-v8a
    }
}

ios {
    QMAKE_INFO_PLIST = ios/Project-Info.plist
    OTHER_FILES += $$QMAKE_INFO_PLIST
}

# set application icons for win and macx
win32 {
    RC_FILE += win/app_icon.rc

    LIBS += -LC:/opencv/build/x86/mingw/lib

    LIBS += -lopencv_core410 \
            -lopencv_imgproc410 \
            -lopencv_objdetect410
}
macx {
    ICON = macx/app_icon.icns
}

