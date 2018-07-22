package com.opsis.opsis2.ui.camera;

import android.content.Context;
import android.util.AttributeSet;
import android.util.Log;
import android.widget.ImageView;

import com.opsis.opsis2.FaceTrackerActivity;


/**
 * Created by Batya on 02.08.2017.
 */

public class SquareImageViewHorizontal extends ImageView {

    public SquareImageViewHorizontal(Context context) {
        super(context);
    }

    public SquareImageViewHorizontal(Context context, AttributeSet attrs) {
        super(context, attrs);
    }

    public SquareImageViewHorizontal(Context context, AttributeSet attrs, int defStyle) {
        super(context, attrs, defStyle);
    }

    @Override
    protected void onMeasure(int widthMeasureSpec, int heightMeasureSpec) {
        super.onMeasure(widthMeasureSpec, heightMeasureSpec);

        int width = getMeasuredWidth();
        setMeasuredDimension(width, width);
        FaceTrackerActivity.GraphSide = width;
    }

}