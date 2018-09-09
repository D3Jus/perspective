package si.fri.jus.perspective;

import android.Manifest;
import android.content.pm.PackageManager;
import android.os.Bundle;
import android.support.design.widget.FloatingActionButton;
import android.support.v4.app.ActivityCompat;
import android.support.v7.app.AppCompatActivity;
import android.util.Log;
import android.view.SurfaceView;
import android.view.View;
import android.view.WindowManager;
import android.widget.Toast;

import org.opencv.android.BaseLoaderCallback;
import org.opencv.android.CameraBridgeViewBase;
import org.opencv.android.LoaderCallbackInterface;
import org.opencv.android.OpenCVLoader;
import org.opencv.core.Mat;

public class MainActivity extends AppCompatActivity implements CameraBridgeViewBase.CvCameraViewListener2, View.OnClickListener {

    private static final String TAG = "Perspective";
    private CameraBridgeViewBase _cameraBridgeViewBase;

    private static AppState appState = AppState.STOPPED;
    private static Mat prevMat = null;

    private FloatingActionButton mainButton;
    private FloatingActionButton pauseButton;

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
        getWindow().addFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN);

        setContentView(R.layout.activity_main);

        ActivityCompat.requestPermissions(MainActivity.this,
                new String[]{Manifest.permission.CAMERA},
                1);

        _cameraBridgeViewBase = findViewById(R.id.main_surface);
        _cameraBridgeViewBase.setVisibility(SurfaceView.VISIBLE);
        _cameraBridgeViewBase.setCvCameraViewListener(this);

        // set buttons
        mainButton = findViewById(R.id.main);
        pauseButton = findViewById(R.id.pause);
        // add on click listeners
        mainButton.setOnClickListener(this);
        pauseButton.setOnClickListener(this);
    }

    @Override
    public void onClick(View view) {
        switch (view.getId()) {
            case R.id.main: {
                // stop or reset after pause
                if(appState.equals(AppState.RUNNING) || appState.equals(AppState.PAUSED)) {
                    mainButton.setImageResource(R.drawable.baseline_play_arrow_white_48);
                    pauseButton.setVisibility(View.GONE);
                    appState = AppState.STOPPED;
                    reset();
                    prevMat = null;
                }
                // start
                else if(appState.equals(AppState.STOPPED)){
                    mainButton.setImageResource(R.drawable.baseline_stop_white_48);
                    pauseButton.setVisibility(View.VISIBLE);
                    appState = AppState.RUNNING;
                }
                break;
            }
            case R.id.pause: {
                mainButton.setImageResource(R.drawable.baseline_clear_white_48);
                pauseButton.setVisibility(View.GONE);
                appState = AppState.PAUSED;
                break;
            }
        }
    }

    private BaseLoaderCallback _baseLoaderCallback = new BaseLoaderCallback(this) {
        @Override
        public void onManagerConnected(int status) {
            switch (status) {
                case LoaderCallbackInterface.SUCCESS: {
                    System.loadLibrary("native-lib");
                    _cameraBridgeViewBase.enableView();
                }
                break;
                default: {
                    super.onManagerConnected(status);
                }
            }
        }
    };

    @Override
    public void onPause() {
        super.onPause();
        disableCamera();
    }

    @Override
    public void onResume() {
        super.onResume();
        if (!OpenCVLoader.initDebug()) {
            Log.d(TAG, "Internal OpenCV library not found. Using OpenCV Manager for initialization");
            OpenCVLoader.initAsync(OpenCVLoader.OPENCV_VERSION_3_0_0, this, _baseLoaderCallback);
        } else {
            Log.d(TAG, "OpenCV library found inside package. Using it!");
            _baseLoaderCallback.onManagerConnected(LoaderCallbackInterface.SUCCESS);
        }
    }

    @Override
    public void onRequestPermissionsResult(int requestCode, String permissions[], int[] grantResults) {
        switch (requestCode) {
            case 1: {
                // If request is cancelled, the result arrays are empty.
                if (!(grantResults.length > 0
                        && grantResults[0] == PackageManager.PERMISSION_GRANTED)) {
                    Toast.makeText(MainActivity.this, "Permission to access camera denied", Toast.LENGTH_SHORT).show();
                }
                break;
            }
        }
    }

    public void onDestroy() {
        super.onDestroy();
        disableCamera();
    }

    public void disableCamera() {
        if (_cameraBridgeViewBase != null)
            _cameraBridgeViewBase.disableView();
    }

    public void onCameraViewStarted(int width, int height) {
        reset();
    }

    public void onCameraViewStopped() {
    }

    public Mat onCameraFrame(CameraBridgeViewBase.CvCameraViewFrame inputFrame) {
        Mat mat = inputFrame.rgba();

        if(appState.equals(AppState.RUNNING)) {
            perspective(mat.getNativeObjAddr());
        } else if(appState.equals(AppState.PAUSED)) {
            if(prevMat == null) {
                perspective(mat.getNativeObjAddr());
                prevMat = mat.clone();
            }
            return prevMat;
        }

        return mat;
    }

    public native void perspective(long matAddrGray);

    public native void reset();
}
