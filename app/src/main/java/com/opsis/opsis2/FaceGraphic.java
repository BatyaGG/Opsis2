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
class FaceGraphic extends GraphicOverlay.Graphic {

    // Used to load the 'native-lib' library on application startup.
    static {
        System.loadLibrary("native-lib");
    }

    private static final float FACE_POSITION_RADIUS = 10.0f;
    private static final float ID_TEXT_SIZE = 40.0f;
    private static final float ID_Y_OFFSET = 50.0f;
    private static final float ID_X_OFFSET = -50.0f;
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

    FaceGraphic(GraphicOverlay overlay) {
        super(overlay);

        mCurrentColorIndex = (mCurrentColorIndex + 1) % COLOR_CHOICES.length;
        final int selectedColor = COLOR_CHOICES[mCurrentColorIndex];

        mFacePositionPaint = new Paint();
        mFacePositionPaint.setColor(selectedColor);
        Log.e("FACE GRAPH", " " + selectedColor);
        mIdPaint = new Paint();
        mIdPaint.setColor(selectedColor);
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
    void updateFace(Face face) {
        mFace = face;
        postInvalidate();
    }

    void updateFrame(Bitmap frameBitmap){
        thisFrameBitmap = frameBitmap;
    }

    void updateCheck(boolean bool) {
        checkShowPoints = bool;
    }

    /**
     * Draws the face annotations for position on the supplied canvas.
     */
    @Override
    public void draw(Canvas canvas) {

        Face face = mFace;
        if (face == null) {
            return;
        }

        ratio = face.getWidth()/canvas.getWidth();
        RatioWeNeed = 0.810867f*ratio + 0.011278f;
        WidthWeNeed = RatioWeNeed*canvas.getWidth();

        // Draws a circle at the position of the detected face, with the face's track id below.
//        float x = translateX(face.getPosition().x + face.getWidth() / 2);
//        float y = translateY(face.getPosition().y + face.getHeight() / 2);

        x = translateX(face.getPosition().x + ((face.getWidth()-WidthWeNeed)/2) + WidthWeNeed/2);
        y = translateY(((face.getHeight() - face.getWidth()) + face.getPosition().y + ((face.getWidth()-WidthWeNeed)/2)) + WidthWeNeed/2);

//        canvas.drawCircle(x, y, FACE_POSITION_RADIUS, mFacePositionPaint);
//        canvas.drawText("id: " + mFaceId, x + ID_X_OFFSET, y + ID_Y_OFFSET, mIdPaint);
//        canvas.drawText("happiness: " + String.format("%.2f", face.getIsSmilingProbability()), x - ID_X_OFFSET, y - ID_Y_OFFSET, mIdPaint);
//        canvas.drawText("right eye: " + String.format("%.2f", face.getIsRightEyeOpenProbability()), x + ID_X_OFFSET * 2, y + ID_Y_OFFSET * 2, mIdPaint);
//        canvas.drawText("left eye: " + String.format("%.2f", face.getIsLeftEyeOpenProbability()), x - ID_X_OFFSET*2, y - ID_Y_OFFSET*2, mIdPaint);

        // Draws a bounding box around the face.
//        float xOffset = scaleX(face.getWidth() / 2.0f);
//        float yOffset = scaleY(face.getHeight() / 2.0f);

        xOffset = scaleX(WidthWeNeed / 2.0f);
        yOffset = scaleY(WidthWeNeed / 2.0f);

        left = x - xOffset;
//        left = left  + ((face.getWidth()-WidthWeNeed)/2);
        top = y - yOffset;
//        top = (face.getHeight() - face.getWidth()) + top + ((face.getWidth()-WidthWeNeed)/2);

        right = x + xOffset;
        bottom = y + yOffset;

        correctionLeft = scaleX(WidthWeNeed*0.022603f);
        correctionTop = scaleY(WidthWeNeed*0.028311f);
        correctionRight = scaleX(WidthWeNeed*0.014210f);
        correctionBot = scaleY(WidthWeNeed*0.036705f);

        left = left + correctionLeft;
        top = top - correctionTop;
        right = right + correctionRight;
        bottom = bottom - correctionBot;

        uX = face.getPosition().x + ((face.getWidth()-WidthWeNeed)/2) + WidthWeNeed/2;
        uY = ((face.getHeight() - face.getWidth()) + face.getPosition().y + ((face.getWidth()-WidthWeNeed)/2)) + WidthWeNeed/2;
        uXOffset = WidthWeNeed / 2.0f;
        uYOffset = WidthWeNeed / 2.0f;
        uLeft = uX - uXOffset;
        uTop = uY - uYOffset;
        uRight = uX + uXOffset;
        uBottom = uY + uYOffset;
        uCorrectionLeft = WidthWeNeed*0.022603f;
        uCorrectionTop = WidthWeNeed*0.028311f;
        uCorrectionRight = WidthWeNeed*0.014210f;
        uCorrectionBot = WidthWeNeed*0.036705f;
        uLeft = uLeft + uCorrectionLeft;
        uTop = uTop - uCorrectionTop;
        uRight = uRight + uCorrectionRight;
        uBottom = uBottom - uCorrectionBot;

        Log.e("Bitmap width", "" + thisFrameBitmap.getWidth());
        Log.e("BItmap height", "" + thisFrameBitmap.getHeight());
        Log.e("Math.round(left)", "" + Math.round(uLeft));
        Log.e("Math.round(top)", "" + Math.round(uTop));
        Log.e("Math.round(right-left)", "" + Math.round(uRight - uLeft));
        Log.e("Math.round(bottom-top)", "" + Math.round(uBottom - uTop));

        if (uLeft < 0){
            uLeft = 0;
        }

        if (uTop < 0){
            uTop = 0;
        }

        if (uRight > thisFrameBitmap.getWidth()){
            uRight = thisFrameBitmap.getWidth();
        }

        if (uBottom > thisFrameBitmap.getHeight()){
            uBottom = thisFrameBitmap.getHeight();
        }

        faceBitmap = Bitmap.createBitmap(thisFrameBitmap, Math.round(uLeft), Math.round(uTop), Math.round(uRight-uLeft), Math.round(uBottom - uTop));

        //                    Saving Images
//            String fname = "Image"+ System.currentTimeMillis() +".jpg";
//            File file = new File(Environment.getExternalStorageDirectory(), fname);
//            if (file.exists()) file.delete();
//            try {
//                FileOutputStream out = new FileOutputStream(file);
//                faceBitmap.compress(Bitmap.CompressFormat.JPEG, 90, out);
//                out.flush();
//                out.close();
////                    Toast.makeText(getApplicationContext(), "image saved", Toast.LENGTH_LONG).show();
//            } catch (Exception e) {
////                    Toast.makeText(getApplicationContext(), "image not saved", Toast.LENGTH_LONG).show();
//                e.printStackTrace();
//            }

//        float right = left + WidthWeNeed;
//        float bottom = top + WidthWeNeed;


        Utils.bitmapToMat(faceBitmap, imgMAT);
        String res = stringFromJNI(imgMAT.getNativeObjAddr(), pointsMAT.getNativeObjAddr());
        Log.e("RESULT", res);
//        element = pointsMAT.get(0,0);
//        Log.e("POINTS", " " + element[0]);

        if (checkShowPoints) {
            for (int i = 0; i < 49; i++) {
                elementX = pointsMAT.get(0, i);
                elementY = pointsMAT.get(1, i);
                canvas.drawCircle(right - scaleX((float) elementX[0]) - correctionRight - correctionLeft, top + scaleY((float) elementY[0]), face.getWidth() * 0.02f, mFacePositionPaint);
            }
        }
//        canvas.drawText(res, left, top, mIdPaint);

//        y = top + 40;
//        x = left;
        counter = 0;
        for (String line: res.split("\n")) {
            if (counter == 0) resultObject[counter] = new Integer(Integer.parseInt(line));
            else if (counter == 7) resultObject[counter] = new String(line);
            else resultObject[counter] = new Double(Double.parseDouble(line));
//            canvas.drawText(line, x, y, mIdPaint);
//            y += mIdPaint.descent() - mIdPaint.ascent();
            counter++;
        }
//        canvas.drawRect(left, top, right, bottom, mBoxPaint);
        Log.e("RESULT OBJECT", "" + resultObject[1] + " " + resultObject[2]);
//        canvas.drawCircle(0, 0, 0.2f, mFacePositionPaint);
        FaceTrackerActivity.Arousal = (double)resultObject[1];
        FaceTrackerActivity.Valence = (double)resultObject[2];
    }
    
    public native String stringFromJNI(long ImageAddress, long facePointsAddress);
}