import Felgo 3.0
import QtQuick 2.12
import QtQuick.Controls 2.12
import QtMultimedia 5.13
import CVFilter 1.0

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

            var contentRect = output.contentRect;

            for(let i = 0; i < rects.length; i++){

                var boundingBox = boundingBoxesHolder.itemAt(i);

                var r = {
                    x: rects[i].rX * contentRect.width,
                    y: rects[i].rY * contentRect.height,
                    width: rects[i].rWidth * contentRect.width,
                    height: rects[i].rHeight * contentRect.height
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
        focus : visible
        fillMode: VideoOutput.PreserveAspectCrop
        filters: [cvFilter]
        autoOrientation: true

        //bounding boxes parent
        Item {
            width: output.contentRect.width
            height: output.contentRect.height
            anchors.centerIn: parent

            Repeater{
                id: boundingBoxesHolder
                model: 20

                Rectangle{
                    border.width: 3
                    border.color: "#2BE982"
                    visible: false
                    color: "transparent"
                    radius: 10
                }
            }
        }

    }

}
