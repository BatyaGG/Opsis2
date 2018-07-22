package com.opsis.opsis2;

import android.Manifest;
import android.app.Activity;
import android.app.AlertDialog;
import android.app.Dialog;
import android.content.Context;
import android.content.DialogInterface;
import android.content.pm.PackageManager;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.ImageFormat;
import android.graphics.Matrix;
import android.graphics.Rect;
import android.graphics.YuvImage;
import android.os.Bundle;
import android.os.Environment;
import android.support.design.widget.AppBarLayout;
import android.support.design.widget.Snackbar;
import android.support.v4.app.ActivityCompat;
import android.support.v7.app.AppCompatActivity;
import android.util.DisplayMetrics;
import android.util.Log;
import android.util.SparseArray;
import android.view.View;
import android.view.ViewGroup;
import android.widget.CheckBox;
import android.widget.ImageView;
import android.widget.RelativeLayout;

import com.google.android.gms.common.ConnectionResult;
import com.google.android.gms.common.GoogleApiAvailability;
import com.google.android.gms.vision.CameraSource;
import com.google.android.gms.vision.Detector;
import com.google.android.gms.vision.Frame;
import com.google.android.gms.vision.MultiProcessor;
import com.google.android.gms.vision.Tracker;
import com.google.android.gms.vision.face.Face;
import com.google.android.gms.vision.face.FaceDetector;
import com.opsis.opsis2.ui.camera.CameraSourcePreview;
import com.opsis.opsis2.ui.camera.CircleOverlay;
import com.opsis.opsis2.ui.camera.GraphicOverlay;
import com.opsis.opsis2.ui.camera.SquareImageView;


import java.io.ByteArrayOutputStream;
import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;

public class FaceTrackerActivity extends AppCompatActivity {

    // Used to load the 'native-lib' library on application startup.
    static {
        System.loadLibrary("native-lib");
    }

    private static final String TAG = "FaceTracker";
    //    This is camera to be best in terms of fps for specified google detector
    private CameraSource mCameraSource = null;

    private CameraSourcePreview mPreview;
    private GraphicOverlay mGraphicOverlay;
    private CircleOverlay mCircleOverlay;

    private CheckBox chkIos;
    private boolean showPoints;

    private static final int RC_HANDLE_GMS = 9001;
    // permission request codes need to be < 256
    private static final int RC_HANDLE_CAMERA_PERM = 2;

    private Bitmap frameBitmap;

    public static int GraphSide;

    public static double Arousal;
    public static double Valence;



    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.main);

        File appFolder = this.getFilesDir();

//        File appFolder = new File(Environment.getExternalStorageDirectory(), "OPSIS_TOWNO");
//        if (!appFolder.exists()) {
//            appFolder.mkdirs();
//        }
//
////        Log.e("PATH TO INTERNAL FOLDER",this.getFilesDir().getAbsolutePath());

        initializeFiles(appFolder);
        initialize(appFolder.getAbsolutePath() + "/");

        mPreview = (CameraSourcePreview) findViewById(R.id.preview);
        mGraphicOverlay = (GraphicOverlay) findViewById(R.id.faceOverlay);
        mCircleOverlay = (CircleOverlay) findViewById(R.id.circleOverlay);


//        DisplayMetrics displayMetrics = this.getResources().getDisplayMetrics();
//        float dpHeight = displayMetrics.heightPixels / displayMetrics.density;
//        float dpWidth = displayMetrics.widthPixels / displayMetrics.density;
//
//        phoneHeight = this.getResources().getDisplayMetrics().heightPixels;
//        Log.e("PHONE HEIGHT", ""+ dpHeight);

//        CameraMother = (RelativeLayout) findViewById(R.id.cameraMother);
//        ViewGroup.LayoutParams params = CameraMother.getLayoutParams();
//        params.height = 480;
//        CameraMother.setLayoutParams(params);

        // Check for the camera permission before accessing the camera.  If the
        // permission is not granted yet, request permission.
        int rc = ActivityCompat.checkSelfPermission(this, Manifest.permission.CAMERA);
        if (rc == PackageManager.PERMISSION_GRANTED) {
            createCameraSource();
        } else {
            requestCameraPermission();
        }

        onCheckboxClicked();



    }

    private void initializeFiles(File appFolder){
        InputStream SDM = getResources().openRawResource(R.raw.sdm_model_detection_tracking);
        InputStream EMO = getResources().openRawResource(R.raw.w_emotion2);
        InputStream FRONT = getResources().openRawResource(R.raw.w_frontalization);
        InputStream POSE = getResources().openRawResource(R.raw.w_pose);
        InputStream FET = getResources().openRawResource(R.raw.feature_template_full49);

        writeFile(appFolder, "SDM_model_detection_tracking.yml", SDM);
        writeFile(appFolder, "W_emotion2.csv", EMO);
        writeFile(appFolder, "W_frontalization.csv", FRONT);
        writeFile(appFolder, "W_pose.csv", POSE);
        writeFile(appFolder, "feature_template_FULL49.csv", FET);
//        writeFile(appFolder, "config.adsc", CONF);

        //        Creating config file
        String textname = "config.adsc";
        File configFile = new File(appFolder, textname);
        if (configFile.exists()) return;
        FileOutputStream out1 = null;
        try{
            out1 = new FileOutputStream(configFile);
        }catch (Exception e){
            e.printStackTrace();
        }

        try {
            String string = "dir," + appFolder.getAbsolutePath()
                    + "\npose,W_pose.csv"
                    + "\nemotion,W_emotion2.csv"
                    + "\nfrontalization,W_frontalization.csv"
                    + "\nmodel,SDM_model_detection_tracking.yml\n";
            out1.write(string.getBytes());
            out1.flush();
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    private void writeFile(File folder, String newFileName, InputStream stream){

        File fileSDM = new File(folder, newFileName);
        if (fileSDM.exists()) return;
        FileOutputStream outStream = null;
        try{
            outStream = new FileOutputStream(fileSDM);
        }catch (Exception e){
            e.printStackTrace();
        }
        try {
            try {
                byte[] buffer = new byte[4 * 1024]; // or other buffer size
                int read;

                while ((read = stream.read(buffer)) != -1) {
                    outStream.write(buffer, 0, read);
                }
                outStream.flush();
            } finally {
                outStream.close();
            }
        } catch (Exception e) {
            e.printStackTrace(); // handle exception, define IOException and others
        }
    }

    public void onCheckboxClicked() {
        chkIos = (CheckBox) findViewById(R.id.showPoints);
        chkIos.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                //is chkIos checked?
                if (((CheckBox) v).isChecked()) {
                    showPoints = true;
                }else{
                    showPoints = false;
                }

            }
        });
    }


    /**
     * A native method that is implemented by the 'native-lib' native library,
     * which is packaged with this application.
     */
    private void requestCameraPermission() {
        Log.w(TAG, "Camera permission is not granted. Requesting permission");

        final String[] permissions = new String[]{Manifest.permission.CAMERA};

        if (!ActivityCompat.shouldShowRequestPermissionRationale(this,
                Manifest.permission.CAMERA)) {
            ActivityCompat.requestPermissions(this, permissions, RC_HANDLE_CAMERA_PERM);
            return;
        }

        final Activity thisActivity = this;

        View.OnClickListener listener = new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                ActivityCompat.requestPermissions(thisActivity, permissions,
                        RC_HANDLE_CAMERA_PERM);
            }
        };

        Snackbar.make(mGraphicOverlay, R.string.permission_camera_rationale,
                Snackbar.LENGTH_INDEFINITE)
                .setAction(R.string.ok, listener)
                .show();
    }

    public static Bitmap RotateBitmap(Bitmap source, float angle)
    {
        Matrix matrix = new Matrix();
        matrix.postRotate(angle);
        return Bitmap.createBitmap(source, 0, 0, source.getWidth(), source.getHeight(), matrix, true);
    }

    class MyFaceDetector extends Detector<Face> {
        private Detector<Face> mDelegate;

        MyFaceDetector(Detector<Face> delegate) {
            mDelegate = delegate;
        }

        public SparseArray<Face> detect(Frame frame) {
            YuvImage yuvImage = new YuvImage(frame.getGrayscaleImageData().array(), ImageFormat.NV21, frame.getMetadata().getWidth(), frame.getMetadata().getHeight(), null);
            ByteArrayOutputStream byteArrayOutputStream = new ByteArrayOutputStream();
            yuvImage.compressToJpeg(new Rect(0, 0, frame.getMetadata().getWidth(), frame.getMetadata().getHeight()), 100, byteArrayOutputStream);
            byte[] jpegArray = byteArrayOutputStream.toByteArray();
            frameBitmap = BitmapFactory.decodeByteArray(jpegArray, 0, jpegArray.length);
            if (frame.getMetadata().getRotation() == 3){
                frameBitmap = RotateBitmap(frameBitmap, -90);
            }else if (frame.getMetadata().getRotation() == 2){
                frameBitmap = RotateBitmap(frameBitmap, 180);
            }else if (frame.getMetadata().getRotation() == 1){
                frameBitmap = RotateBitmap(frameBitmap, 90);
            }
            return mDelegate.detect(frame);
        }

        public boolean isOperational() {
            return mDelegate.isOperational();
        }

        public boolean setFocus(int id) {
            return mDelegate.setFocus(id);
        }
    }

    /**
     * Creates and starts the camera.  Note that this uses a higher resolution in comparison
     * to other detection examples to enable the barcode detector to detect small barcodes
     * at long distances.
     */
    private void createCameraSource() {

        Context context = getApplicationContext();
        FaceDetector detector = new FaceDetector.Builder(context)
                .setClassificationType(FaceDetector.ALL_CLASSIFICATIONS)
                .setProminentFaceOnly(true)
                .build();

        MyFaceDetector myFaceDetector = new MyFaceDetector(detector);

        myFaceDetector.setProcessor(
                new MultiProcessor.Builder<>(new GraphicFaceTrackerFactory())
                        .build());

        if (!myFaceDetector.isOperational()) {
            // Note: The first time that an app using face API is installed on a device, GMS will
            // download a native library to the device in order to do detection.  Usually this
            // completes before the app is run for the first time.  But if that download has not yet
            // completed, then the above call will not detect any faces.
            //
            // isOperational() can be used to check if the required native library is currently
            // available.  The detector will automatically become operational once the library
            // download completes on device.
            Log.w(TAG, "Face detector dependencies are not yet available.");
        }

        mCameraSource = new CameraSource.Builder(context, myFaceDetector)
                .setRequestedPreviewSize(640, 480)
                .setFacing(CameraSource.CAMERA_FACING_FRONT)
                .setRequestedFps(30.0f)
                .build();
    }

    /**
     * Restarts the camera.
     */
    @Override
    protected void onResume() {
        super.onResume();
        startCameraSource();
    }

    /**
     * Stops the camera.
     */
    @Override
    protected void onPause() {
        super.onPause();
        mPreview.stop();
    }

    /**
     * Releases the resources associated with the camera source, the associated detector, and the
     * rest of the processing pipeline.
     */
    @Override
    protected void onDestroy() {
        super.onDestroy();
        if (mCameraSource != null) {
            mCameraSource.release();
        }
    }

    /**
     * Callback for the result from requesting permissions. This method
     * is invoked for every call on {@link #requestPermissions(String[], int)}.
     * <p>
     * <strong>Note:</strong> It is possible that the permissions request interaction
     * with the user is interrupted. In this case you will receive empty permissions
     * and results arrays which should be treated as a cancellation.
     * </p>
     *
     * @param requestCode  The request code passed in {@link #requestPermissions(String[], int)}.
     * @param permissions  The requested permissions. Never null.
     * @param grantResults The grant results for the corresponding permissions
     *                     which is either {@link PackageManager#PERMISSION_GRANTED}
     *                     or {@link PackageManager#PERMISSION_DENIED}. Never null.
     * @see #requestPermissions(String[], int)
     */
    @Override
    public void onRequestPermissionsResult(int requestCode, String[] permissions, int[] grantResults) {
        if (requestCode != RC_HANDLE_CAMERA_PERM) {
            Log.d(TAG, "Got unexpected permission result: " + requestCode);
            super.onRequestPermissionsResult(requestCode, permissions, grantResults);
            return;
        }

        if (grantResults.length != 0 && grantResults[0] == PackageManager.PERMISSION_GRANTED) {
            Log.d(TAG, "Camera permission granted - initialize the camera source");
            // we have permission, so create the camerasource
            createCameraSource();
            return;
        }

        Log.e(TAG, "Permission not granted: results len = " + grantResults.length +
                " Result code = " + (grantResults.length > 0 ? grantResults[0] : "(empty)"));

        DialogInterface.OnClickListener listener = new DialogInterface.OnClickListener() {
            public void onClick(DialogInterface dialog, int id) {
                finish();
            }
        };

        AlertDialog.Builder builder = new AlertDialog.Builder(this);
        builder.setTitle("Face Tracker sample")
                .setMessage(R.string.no_camera_permission)
                .setPositiveButton(R.string.ok, listener)
                .show();
    }

    //==============================================================================================
    // Camera Source Preview
    //==============================================================================================

    /**
     * Starts or restarts the camera source, if it exists.  If the camera source doesn't exist yet
     * (e.g., because onResume was called before the camera source was created), this will be called
     * again when the camera source is created.
     */
    private void startCameraSource() {

        // check that the device has play services available.
        int code = GoogleApiAvailability.getInstance().isGooglePlayServicesAvailable(
                getApplicationContext());
        if (code != ConnectionResult.SUCCESS) {
            Dialog dlg =
                    GoogleApiAvailability.getInstance().getErrorDialog(this, code, RC_HANDLE_GMS);
            dlg.show();
        }

        if (mCameraSource != null) {
            try {
                mPreview.start(mCameraSource, mGraphicOverlay);
            } catch (IOException e) {
                Log.e(TAG, "Unable to start camera source.", e);
                mCameraSource.release();
                mCameraSource = null;
            }
        }
    }

    //==============================================================================================
    // Graphic Face Tracker
    //==============================================================================================

    /**
     * Factory for creating a face tracker to be associated with a new face.  The multiprocessor
     * uses this factory to create face trackers as needed -- one for each individual.
     */
    private class GraphicFaceTrackerFactory implements MultiProcessor.Factory<Face> {
        @Override
        public Tracker<Face> create(Face face) {
            return new GraphicFaceTracker(mGraphicOverlay, mCircleOverlay);
        }
    }

    /**
     * Face tracker for each detected individual. This maintains a face graphic within the app's
     * associated face overlay.
     */
    private class GraphicFaceTracker extends Tracker<Face> {


        private GraphicOverlay mOverlay;
        private FaceGraphic mFaceGraphic;

        private CircleOverlay mCOverlay;
        private CircleGraphic mCircleGraphic;

        GraphicFaceTracker(GraphicOverlay overlay, CircleOverlay Coverlay) {
            mOverlay = overlay;
            mFaceGraphic = new FaceGraphic(overlay);

            mCOverlay = Coverlay;
            mCircleGraphic = new CircleGraphic(Coverlay);
            mCOverlay.setTransformationInfo(GraphSide, GraphSide);
            Log.e("CIRCLE OVERLAY VALUES", "" + GraphSide);
//            mCOverlay.setTransformationInfo();
        }

        /**
         * Start tracking the detected face instance within the face overlay.
         */
        @Override
        public void onNewItem(int faceId, Face item) {
            mFaceGraphic.setId(faceId);
            mCircleGraphic.setId(faceId);
        }

        /**
         * Update the position/characteristics of the face within the overlay.
         */
        @Override
        public void onUpdate(FaceDetector.Detections<Face> detectionResults, Face face) {

            mOverlay.add(mFaceGraphic);
            mFaceGraphic.updateFace(face);
            mFaceGraphic.updateFrame(frameBitmap);
            mFaceGraphic.updateCheck(showPoints);

            mCOverlay.add(mCircleGraphic);
            mCircleGraphic.updateValues(Arousal, Valence);

//            String fname = "Image"+ 999 +".jpg";
//                    File file = new File(Environment.getExternalStorageDirectory(), fname);
//                    if (file.exists()) file.delete();
//                        try {
//                            FileOutputStream out = new FileOutputStream(file);
//                            TempBitmap.compress(Bitmap.CompressFormat.JPEG, 90, out);
//                            out.flush();
//                            out.close();
//                            Toast.makeText(getApplicationContext(), "image saved", Toast.LENGTH_LONG).show();
//                        } catch (Exception e) {
//                            Toast.makeText(getApplicationContext(), "image not saved", Toast.LENGTH_LONG).show();
//                            e.printStackTrace();
//                        }
        }

        /**
         * Hide the graphic when the corresponding face was not detected.  This can happen for
         * intermediate frames temporarily (e.g., if the face was momentarily blocked from
         * view).
         */
        @Override
        public void onMissing(FaceDetector.Detections<Face> detectionResults) {
            mOverlay.remove(mFaceGraphic);
            mCOverlay.remove(mCircleGraphic);
        }

        /**
         * Called when the face is assumed to be gone for good. Remove the graphic annotation from
         * the overlay.
         */
        @Override
        public void onDone() {
            mOverlay.remove(mFaceGraphic);
            mCOverlay.remove(mCircleGraphic);
        }
    }

    public native void initialize(String appFolder);
}
