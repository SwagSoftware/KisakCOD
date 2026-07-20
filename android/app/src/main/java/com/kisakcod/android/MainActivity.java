package com.kisakcod.android;

import android.app.Activity;
import android.os.Bundle;
import android.view.Gravity;
import android.view.KeyEvent;
import android.view.View;
import android.view.Window;
import android.view.WindowInsets;
import android.view.WindowInsetsController;
import android.view.WindowManager;
import android.widget.Button;
import android.widget.FrameLayout;
import android.widget.LinearLayout;
import android.widget.ScrollView;
import android.widget.TextView;

public final class MainActivity extends Activity {
    static {
        System.loadLibrary("kisakcod_android");
    }

    private static native String nativeBootstrapStatus(String dataDirectory);
    static native void nativeSurfaceCreated(Object surface);
    static native void nativeSurfaceChanged(Object surface, int width, int height);
    static native void nativeSurfaceDestroyed();
    static native void nativeTouch(float x, float y, int action);
    static native boolean nativeKey(int unicodeChar, int keyCode);
    static native void nativeBack();

    private String dataPath;
    private TextView status;
    private View diagnosticPanel;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        requestWindowFeature(Window.FEATURE_NO_TITLE);
        getWindow().addFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN | WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
        hideSystemBars();

        dataPath = getExternalFilesDir(null) != null
            ? getExternalFilesDir(null).getAbsolutePath() + "/cod4"
            : getFilesDir().getAbsolutePath() + "/cod4";

        FrameLayout root = new FrameLayout(this);
        root.setBackgroundColor(0xff060805);

        GameSurfaceView surface = new GameSurfaceView(this);
        root.addView(surface, new FrameLayout.LayoutParams(
            FrameLayout.LayoutParams.MATCH_PARENT,
            FrameLayout.LayoutParams.MATCH_PARENT
        ));

        View diagnosticHotspot = new View(this);
        diagnosticHotspot.setBackgroundColor(0x00000000);
        FrameLayout.LayoutParams toggleParams = new FrameLayout.LayoutParams(dp(56), dp(56));
        toggleParams.gravity = Gravity.TOP | Gravity.END;

        LinearLayout panel = new LinearLayout(this);
        panel.setOrientation(LinearLayout.VERTICAL);
        panel.setPadding(dp(16), dp(12), dp(16), dp(12));
        panel.setBackgroundColor(0xcc060805);
        panel.setVisibility(View.GONE);
        diagnosticPanel = panel;

        ScrollView statusScroll = new ScrollView(this);
        statusScroll.setFillViewport(false);
        LinearLayout.LayoutParams statusScrollParams = new LinearLayout.LayoutParams(
            LinearLayout.LayoutParams.MATCH_PARENT,
            dp(150)
        );

        status = new TextView(this);
        status.setTextColor(0xffc6bda9);
        status.setTextSize(12);
        status.setGravity(Gravity.START);

        Button rescan = new Button(this);
        rescan.setText("Relancer le diagnostic");
        rescan.setAllCaps(false);
        rescan.setOnClickListener(view -> runBootstrapProbeAsync(true));

        TextView note = new TextView(this);
        note.setText(
            "Dossier COD4: "
                + dataPath
                + "\\nNe mettez pas ces fichiers dans Git ou dans l'APK."
        );
        note.setTextColor(0xff8f9b7a);
        note.setTextSize(12);
        note.setGravity(Gravity.START);
        note.setPadding(0, dp(8), 0, 0);

        statusScroll.addView(status);
        panel.addView(statusScroll, statusScrollParams);
        panel.addView(rescan);
        panel.addView(note);
        FrameLayout.LayoutParams panelParams = new FrameLayout.LayoutParams(
            FrameLayout.LayoutParams.MATCH_PARENT,
            FrameLayout.LayoutParams.WRAP_CONTENT
        );
        panelParams.gravity = Gravity.BOTTOM;

        diagnosticHotspot.setOnClickListener(view -> {
            diagnosticPanel.setVisibility(diagnosticPanel.getVisibility() == View.VISIBLE ? View.GONE : View.VISIBLE);
            hideSystemBars();
        });

        root.addView(panel, panelParams);
        root.addView(diagnosticHotspot, toggleParams);
        setContentView(root);
        runBootstrapProbeAsync(false);
    }

    @Override
    public void onWindowFocusChanged(boolean hasFocus) {
        super.onWindowFocusChanged(hasFocus);
        if (hasFocus) {
            hideSystemBars();
        }
    }

    @Override
    public boolean dispatchKeyEvent(KeyEvent event) {
        if (event.getAction() == KeyEvent.ACTION_DOWN) {
            int unicode = event.getUnicodeChar();
            int keyCode = event.getKeyCode();
            if (nativeKey(unicode, keyCode)) {
                return true;
            }
            if (unicode != 0 || keyCode == KeyEvent.KEYCODE_DEL
                || keyCode == KeyEvent.KEYCODE_ENTER || keyCode == KeyEvent.KEYCODE_NUMPAD_ENTER) {
                return true;
            }
        }
        return super.dispatchKeyEvent(event);
    }

    @Override
    public void onBackPressed() {
        if (diagnosticPanel != null && diagnosticPanel.getVisibility() == View.VISIBLE) {
            diagnosticPanel.setVisibility(View.GONE);
        } else {
            nativeBack();
        }
        hideSystemBars();
    }

    private void hideSystemBars() {
        View decorView = getWindow().getDecorView();
        if (android.os.Build.VERSION.SDK_INT >= android.os.Build.VERSION_CODES.R) {
            getWindow().setDecorFitsSystemWindows(false);
            WindowInsetsController controller = decorView.getWindowInsetsController();
            if (controller != null) {
                controller.hide(WindowInsets.Type.statusBars() | WindowInsets.Type.navigationBars());
                controller.setSystemBarsBehavior(WindowInsetsController.BEHAVIOR_SHOW_TRANSIENT_BARS_BY_SWIPE);
            }
        } else {
            decorView.setSystemUiVisibility(
                View.SYSTEM_UI_FLAG_FULLSCREEN
                    | View.SYSTEM_UI_FLAG_HIDE_NAVIGATION
                    | View.SYSTEM_UI_FLAG_IMMERSIVE_STICKY
                    | View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN
                    | View.SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION
                    | View.SYSTEM_UI_FLAG_LAYOUT_STABLE
            );
        }
    }

    private String runBootstrapProbe() {
        return nativeBootstrapStatus(dataPath);
    }

    private void runBootstrapProbeAsync(boolean showPanelWhenDone) {
        status.setText("Diagnostic COD4 en cours...");
        if (showPanelWhenDone) {
            diagnosticPanel.setVisibility(View.VISIBLE);
        }
        new Thread(() -> {
            String result = runBootstrapProbe();
            runOnUiThread(() -> {
                status.setText(result);
                diagnosticPanel.setVisibility(
                    showPanelWhenDone || !hasRenderableMenuData(result) ? View.VISIBLE : View.GONE
                );
                hideSystemBars();
            });
        }, "kisak-bootstrap-probe").start();
    }

    private boolean hasRenderableMenuData(String result) {
        return result.contains("Zone loader common.ff:")
            && result.contains("Zone loader ui.ff:")
            && !result.contains("common.ff introuvable")
            && !result.contains("ui.ff introuvable");
    }

    private int dp(int value) {
        return (int) (value * getResources().getDisplayMetrics().density + 0.5f);
    }
}
