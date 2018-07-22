package com.opsis.opsis2.ui.camera;

import android.content.Context;
import android.util.AttributeSet;
import android.widget.RelativeLayout;

/**
 * Created by Batya on 03.08.2017.
 */

public class MyRelativeView extends RelativeLayout {

    public MyRelativeView(Context context) {
        super(context);
    }

    public MyRelativeView(Context context, AttributeSet attrs) {
        super(context, attrs);
    }

    public MyRelativeView(Context context, AttributeSet attrs, int defStyle) {
        super(context, attrs, defStyle);
    }

    @Override
    protected void onMeasure(int widthMeasureSpec, int heightMeasureSpec) {
        super.onMeasure(widthMeasureSpec, heightMeasureSpec);

        int height = getMeasuredHeight();
        setMeasuredDimension(height, height);
    }

}
