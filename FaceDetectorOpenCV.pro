# allows to add DEPLOYMENTFOLDERS and links to the Felgo library and QtCreator auto-completion
CONFIG += felgo

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
    opencvfilter.cpp


android {
    ANDROID_PACKAGE_SOURCE_DIR = $$PWD/android
    OTHER_FILES += android/AndroidManifest.xml       android/build.gradle
}

ios {
    QMAKE_INFO_PLIST = ios/Project-Info.plist
    OTHER_FILES += $$QMAKE_INFO_PLIST
}

# set application icons for win and macx
win32 {
    RC_FILE += win/app_icon.rc

    INCLUDEPATH += C:/opencv/build/include
    LIBS += -LC:/opencv/build/x86/mingw/lib


    LIBS += -lopencv_core410 \
            -lopencv_imgproc410 \
            -lopencv_imgcodecs410 \
            -lopencv_videoio410 \
            -lopencv_flann410 \
            -lopencv_highgui410 \
            -lopencv_features2d410 \
            -lopencv_photo410 \
            -lopencv_video410 \
            -lopencv_calib3d410 \
            -lopencv_objdetect410 \
            -lopencv_stitching410 \
            -lopencv_dnn410 \
            -lopencv_ml410
}
macx {
    ICON = macx/app_icon.icns
}

HEADERS += \
    opencvfilter.h
