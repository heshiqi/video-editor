package com.h.arrow.medialib.mediaplayer;

import android.graphics.Rect;

public class HTimedText {

    private Rect mTextBounds = null;
    private String mTextChars = null;

    public HTimedText(Rect bounds, String text) {
        mTextBounds = bounds;
        mTextChars = text;
    }

    public Rect getBounds() {
        return mTextBounds;
    }

    public String getText() {
        return mTextChars;
    }
}
