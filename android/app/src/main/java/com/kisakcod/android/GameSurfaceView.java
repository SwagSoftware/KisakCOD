package com.kisakcod.android;

import android.content.Context;
import android.view.KeyEvent;
import android.view.MotionEvent;
import android.view.SurfaceHolder;
import android.view.SurfaceView;

final class GameSurfaceView extends SurfaceView implements SurfaceHolder.Callback {
    static final int TOUCH_DOWN = 0;
    static final int TOUCH_MOVE = 1;
    static final int TOUCH_UP = 2;
    static final int TOUCH_CANCEL = 3;

    GameSurfaceView(Context context) {
        super(context);
        getHolder().addCallback(this);
        setFocusable(true);
        setFocusableInTouchMode(true);
        requestFocus();
    }

    @Override
    public void surfaceCreated(SurfaceHolder holder) {
        MainActivity.nativeSurfaceCreated(holder.getSurface());
    }

    @Override
    public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {
        MainActivity.nativeSurfaceChanged(holder.getSurface(), width, height);
    }

    @Override
    public void surfaceDestroyed(SurfaceHolder holder) {
        MainActivity.nativeSurfaceDestroyed();
    }

    @Override
    public boolean onTouchEvent(MotionEvent event) {
        switch (event.getActionMasked()) {
            case MotionEvent.ACTION_DOWN:
                MainActivity.nativeTouch(event.getX(), event.getY(), TOUCH_DOWN);
                return true;
            case MotionEvent.ACTION_MOVE:
                MainActivity.nativeTouch(event.getX(), event.getY(), TOUCH_MOVE);
                return true;
            case MotionEvent.ACTION_UP:
                MainActivity.nativeTouch(event.getX(), event.getY(), TOUCH_UP);
                return true;
            case MotionEvent.ACTION_CANCEL:
                MainActivity.nativeTouch(event.getX(), event.getY(), TOUCH_CANCEL);
                return true;
            default:
                return true;
        }
    }

    @Override
    public boolean onKeyDown(int keyCode, KeyEvent event) {
        int unicode = event.getUnicodeChar();
        if (MainActivity.nativeKey(unicode, keyCode)) {
            return true;
        }
        if (unicode != 0 || keyCode == KeyEvent.KEYCODE_DEL
            || keyCode == KeyEvent.KEYCODE_ENTER || keyCode == KeyEvent.KEYCODE_NUMPAD_ENTER) {
            return true;
        }
        return super.onKeyDown(keyCode, event);
    }
}
