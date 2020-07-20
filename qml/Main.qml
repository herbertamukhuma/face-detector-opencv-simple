import Felgo 3.0
import QtQuick 2.12
import QtQuick.Controls 2.12
import QtMultimedia 5.13
import CVFilter 1.0
import QtSensors 5.11

App {
    width: 900
    height: 600

    property bool drawing: false

    function resetBoundingBoxes() {
        for (var i = 0; i < boundingBoxesHolder.count; ++i)
            boundingBoxesHolder.itemAt(i).visible = false;
    }

    Timer{
        id: drawingTimer
        interval: 100
        onTriggered: {
            drawing = false;
        }
    }

    OrientationSensor{
        active: true

        Component.onCompleted: start()

        onReadingChanged: {

            switch(reading.orientation){
            case OrientationReading.TopUp:
                cvFilter.videoOutputOrientation = 270;
                break;
            case OrientationReading.TopDown:
                cvFilter.videoOutputOrientation = 90;
                break;
            case OrientationReading.LeftUp:
                cvFilter.videoOutputOrientation = 180;
                break;
            case OrientationReading.RightUp:
                cvFilter.videoOutputOrientation = 0;
                break;
            }
        }
    }

    Camera {
        id: camera
        position: Camera.FrontFace
        viewfinder {
            //resolution: "320x240"
            maximumFrameRate: 15
        }
    }

    CVFilter{
        id: cvFilter

        onObjectDetected: {

            if(drawing) return;

            drawing = true;

            resetBoundingBoxes();

            rects = JSON.parse(rects);

            for(let i = 0; i < rects.length; i++){

                var boundingBox = boundingBoxesHolder.itemAt(i);
                //var r = output.mapNormalizedRectToItem(Qt.rect(rects[i].rX, rects[i].rY, rects[i].rWidth, rects[i].rHeight));
                var r = {
                    x: rects[i].rX * output.width,
                    y: rects[i].rY * output.height,
                    width: rects[i].rWidth * output.width,
                    height: rects[i].rHeight * output.height
                };

                boundingBox.x = r.x;
                boundingBox.y = r.y;
                boundingBox.width = r.width;
                boundingBox.height = r.height;
                boundingBox.visible = true;
            }

            drawingTimer.start();

        }
    }

    VideoOutput {
        id: output
        source: camera
        anchors.fill: parent
        focus : visible // to receive focus and capture key events when visible
        fillMode: VideoOutput.PreserveAspectCrop
        filters: [cvFilter]
        autoOrientation: true

        Repeater{
            id: boundingBoxesHolder
            model: 20

            Rectangle{
                border.width: 2
                border.color: "yellow"
                color: "transparent"
                visible: false
            }
        }

        Text {
            width: contentWidth
            height: 20
            anchors.centerIn: parent
            color: "red"
            text: "camera orientation: " + camera.orientation + "\noutput orientation: " + output.orientation
        }

        Button{
            width: 150
            height: 40
            text: "Toggle"
            anchors.bottom: parent.bottom
            anchors.bottomMargin: 30
            anchors.horizontalCenter: parent.horizontalCenter

            onClicked: {
                if(camera.position === Camera.FrontFace){
                    camera.position = Camera.BackFace;
                }else{
                    camera.position = Camera.FrontFace;
                }
            }
        }

    }

}
