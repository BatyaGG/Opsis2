package com.opsis.opsis2.ui.camera;

import android.content.Context;
import android.util.AttributeSet;
import android.util.Log;
import android.widget.ImageView;
import com.opsis.opsis2.FaceTrackerActivity;


/**
 * Created by Batya on 02.08.2017.
 */

public class SquareImageView extends ImageView {

    public static int GraphSide;

    public SquareImageView(Context context) {
        super(context);
    }

    public SquareImageView(Context context, AttributeSet attrs) {
        super(context, attrs);
    }

    public SquareImageView(Context context, AttributeSet attrs, int defStyle) {
        super(context, attrs, defStyle);
    }

    @Override
    protected void onMeasure(int widthMeasureSpec, int heightMeasureSpec) {
        super.onMeasure(widthMeasureSpec, heightMeasureSpec);

        int height = getMeasuredHeight();
        setMeasuredDimension(height, height);
        FaceTrackerActivity.GraphSide = height;
    }

}