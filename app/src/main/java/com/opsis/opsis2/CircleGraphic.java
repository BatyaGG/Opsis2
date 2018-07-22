package com.opsis.opsis2;

import android.graphics.Bitmap;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.os.Bundle;
import android.os.Environment;
import android.support.v7.app.AppCompatActivity;
import android.util.Log;
import android.widget.CheckBox;

import com.google.android.gms.vision.CameraSource;
import com.google.android.gms.vision.face.Face;
import com.opsis.opsis2.ui.camera.CircleOverlay;
import com.opsis.opsis2.ui.camera.GraphicOverlay;

import java.io.File;
import java.io.FileOutputStream;
import com.opsis.opsis2.ui.camera.CameraSourcePreview;
import com.opsis.opsis2.ui.camera.GraphicOverlay;

import org.opencv.android.Utils;
import org.opencv.core.Mat;

/**
 * Graphic instance for rendering face position, orientation, and landmarks within an associated
 * graphic overlay view.
 */
class CircleGraphic extends CircleOverlay.Graphic {

    private static final float FACE_POSITION_RADIUS = 10.0f;
    private static final float ID_TEXT_SIZE = 300.0f;
    private static final float ID_Y_OFFSET = 50.0f;
    private static final float ID_X_OFFSET = 50.0f;
    private static final float BOX_STROKE_WIDTH = 5.0f;
    private Bitmap thisFrameBitmap;
    private Bitmap faceBitmap;
    private float ratio, RatioWeNeed, WidthWeNeed, x, y, xOffset, yOffset, left, top, right, bottom, correctionLeft, correctionTop, correctionRight, correctionBot;
    private float uX, uY, uXOffset, uYOffset, uLeft, uTop, uRight, uBottom, uCorrectionLeft, uCorrectionTop, uCorrectionRight, uCorrectionBot;
    private Object[] resultObject = new Object[9];
    private int counter;
    private Mat imgMAT = new Mat();
    private Mat pointsMAT = new Mat();
    private double [] elementX;
    private double [] elementY;
    private boolean checkShowPoints;

    private static final int COLOR_CHOICES[] = {
            Color.BLUE,
            Color.CYAN,
            Color.GREEN,
            Color.MAGENTA,
            Color.RED,
            Color.WHITE,
            Color.YELLOW
    };
    private static int mCurrentColorIndex = 0;

    private Paint mFacePositionPaint;
    private Paint mIdPaint;
    private Paint mBoxPaint;

    private volatile Face mFace;
    private int mFaceId;
    private float mFaceHappiness;
    private double Arousal;
    private double Valence;

    CircleGraphic(CircleOverlay overlay) {
        super(overlay);

        mCurrentColorIndex = (mCurrentColorIndex + 1) % COLOR_CHOICES.length;
        final int selectedColor = COLOR_CHOICES[mCurrentColorIndex];

        mFacePositionPaint = new Paint();
        mFacePositionPaint.setColor(selectedColor);

        mIdPaint = new Paint();
        mIdPaint.setColor(selectedColor);
        Log.e("CIRCLE GRAPH", " " + selectedColor);
        mIdPaint.setTextSize(ID_TEXT_SIZE);

        mBoxPaint = new Paint();
        mBoxPaint.setColor(selectedColor);
        mBoxPaint.setStyle(Paint.Style.STROKE);
        mBoxPaint.setStrokeWidth(BOX_STROKE_WIDTH);
    }

    void setId(int id) {
        mFaceId = id;
    }

    /**
     * Updates the face instance from the detection of the most recent frame.  Invalidates the
     * relevant portions of the overlay to trigger a redraw.
     */
//    void updateFace(Face face) {
//        mFace = face;
//        postInvalidate();
//    }

//    void updateFrame(Bitmap frameBitmap){
//        thisFrameBitmap = frameBitmap;
//    }
//
//    void updateCheck(boolean bool) {
//        checkShowPoints = bool;
//    }

    void updateValues(double arousal, double valence){
        Arousal = arousal;
        Valence = valence;
    }

    private float ratioX(float percentage){
        int SideLength = FaceTrackerActivity.GraphSide;
        return translateX((SideLength * percentage + SideLength)/2);
    }

    private float ratioY(float percentage){
        int SideLength = FaceTrackerActivity.GraphSide;
        return translateY((SideLength * (-percentage) + SideLength)/2);
    }

    /**
     * Draws the face annotations for position on the supplied canvas.
     */
    @Override
    public void draw(Canvas canvas) {
        canvas.drawCircle(ratioX((float)Valence), ratioY((float)Arousal), FACE_POSITION_RADIUS, mFacePositionPaint);
//        canvas.drawCircle(scaleX(60.0f), scaleY(60.0f), 2f, mFacePositionPaint);
//        canvas.drawText(".", x + ID_X_OFFSET, y + ID_Y_OFFSET, mIdPaint);
    }
}