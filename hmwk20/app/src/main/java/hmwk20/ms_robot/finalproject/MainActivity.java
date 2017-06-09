package hmwk20.ms_robot.finalproject;
import android.Manifest;
import android.app.Activity;
import android.app.PendingIntent;
import android.content.Context;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.graphics.Bitmap;
import android.graphics.Canvas;
import android.graphics.Paint;
import android.graphics.SurfaceTexture;
import android.hardware.Camera;
import android.hardware.usb.UsbDeviceConnection;
import android.hardware.usb.UsbManager;
import android.os.Bundle;
import android.support.v4.content.ContextCompat;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.TextureView;
import android.view.View;
import android.view.WindowManager;
import android.widget.Button;
import android.widget.ScrollView;
import android.widget.TextView;
import android.widget.SeekBar;

import com.hoho.android.usbserial.driver.CdcAcmSerialDriver;
import com.hoho.android.usbserial.driver.ProbeTable;
import com.hoho.android.usbserial.driver.UsbSerialDriver;
import com.hoho.android.usbserial.driver.UsbSerialPort;
import com.hoho.android.usbserial.driver.UsbSerialProber;
import com.hoho.android.usbserial.util.SerialInputOutputManager;

import java.io.IOException;
import java.io.UnsupportedEncodingException;
import java.util.List;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

import static android.R.attr.left;
import static android.R.attr.max;
import static android.graphics.Color.blue;
import static android.graphics.Color.green;
import static android.graphics.Color.red;
import static android.graphics.Color.rgb;
import static android.icu.lang.UCharacter.GraphemeClusterBreak.T;

public class MainActivity extends Activity implements TextureView.SurfaceTextureListener {

    private Camera mCamera;
    private TextureView mTextureView;
    private SurfaceView mSurfaceView;
    private TextView wheelSpeedText;
    private TextView radiusText;
    private TextView maxSpeedText;
    private TextView mTextView;
    private TextView KPText;
    private SurfaceHolder mSurfaceHolder;
    private Bitmap bmp = Bitmap.createBitmap(800, 600, Bitmap.Config.ARGB_8888);
    private Canvas canvas = new Canvas(bmp);
    private Paint paint1 = new Paint();

    private SeekBar myRadius;
    private SeekBar brightBar;
    private SeekBar speedBar;
    private SeekBar KPBar;
    private int radius = 0;
    private int brightness = 0;
    private Button stopButton;
    private Button startButton;
    private double leftWheelSpeed;
    private double rightWheelSpeed;
    private int servoAngle;
    private double KP;

    private boolean carRunning;

    private UsbManager manager;
    private UsbSerialPort sPort;
    private final ExecutorService mExecutor = Executors.newSingleThreadExecutor();
    private SerialInputOutputManager mSerialIoManager;
    private double maxSpeed;

    static long prevtime = 0; // for FPS calculation

    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON); // keeps the screen from turning off

        carRunning = false;

        myRadius = (SeekBar) findViewById(R.id.radiusBar);
        brightBar = (SeekBar) findViewById(R.id.brightnessBar);
        wheelSpeedText = (TextView) findViewById(R.id.speedStatus);
        mTextView = (TextView) findViewById(R.id.cameraStatus);
        radiusText = (TextView) findViewById(R.id.radiusValue);
        maxSpeedText = (TextView) findViewById(R.id.maxSpeedValue);
        speedBar = (SeekBar)findViewById(R.id.maxSpeed);
        KPBar = (SeekBar)findViewById(R.id.KPValue);
        KPText = (TextView)findViewById(R.id.KPText);


        stopButton = (Button)findViewById(R.id.stop);
        startButton = (Button)findViewById(R.id.start);

        leftWheelSpeed = 1;
        rightWheelSpeed = 1;
        servoAngle = 75;
        KP = .25;


        setMyControlListener();
        setMyButtonListener();

        if (ContextCompat.checkSelfPermission(this, Manifest.permission.CAMERA) == PackageManager.PERMISSION_GRANTED) {
            mSurfaceView = (SurfaceView) findViewById(R.id.surfaceview);
            mSurfaceHolder = mSurfaceView.getHolder();

            mTextureView = (TextureView) findViewById(R.id.textureview);
            mTextureView.setSurfaceTextureListener(this);

            // set the paintbrush for writing text on the image
            paint1.setColor(0xffff0000); // red
            paint1.setTextSize(24);

            mTextView.setText("started camera");
        } else {
            mTextView.setText("no camera permissions");
        }

        manager = (UsbManager) getSystemService(Context.USB_SERVICE);

    }

    public void onSurfaceTextureAvailable(SurfaceTexture surface, int width, int height) {
        mCamera = Camera.open();
        Camera.Parameters parameters = mCamera.getParameters();
        parameters.setPreviewSize(640, 480);
        parameters.setFocusMode(Camera.Parameters.FOCUS_MODE_INFINITY); // no autofocusing
        parameters.setAutoExposureLock(false); // keep the white balance constant
        mCamera.setParameters(parameters);
        mCamera.setDisplayOrientation(90); // rotate to portrait mode

        try {
            mCamera.setPreviewTexture(surface);
            mCamera.startPreview();
        } catch (IOException ioe) {
            // Something bad happened
        }
    }

    public void onSurfaceTextureSizeChanged(SurfaceTexture surface, int width, int height) {
        // Ignored, Camera does all the work for us
    }

    public boolean onSurfaceTextureDestroyed(SurfaceTexture surface) {
        mCamera.stopPreview();
        mCamera.release();
        return true;
    }

    // the important function
    public void onSurfaceTextureUpdated(SurfaceTexture surface) {
        // every time there is a new Camera preview frame
        mTextureView.getBitmap(bmp);
        int COM = 0;
        int sum_mr; // the sum of the mass times the radius
        int sum_m; // the sum of the masses
        int height = 400;

        final Canvas c = mSurfaceHolder.lockCanvas();
        if (c != null) {
            int[] pixels = new int[bmp.getWidth()]; // pixels[] is the RGBA data

            // in the row, see if there is more green than red
            sum_m = 0;
            sum_mr = 0;
            bmp.getPixels(pixels, 0, bmp.getWidth(), 0, height, bmp.getWidth(), 1);
            for (int i = 0; i < bmp.getWidth(); i++) {
                if (((green(pixels[i]) - red(pixels[i])) > -radius) && ((green(pixels[i]) - red(pixels[i])) < radius) && (green(pixels[i]) > brightness) &&
                        ((blue(pixels[i]) - red(pixels[i])) > -radius) && ((blue(pixels[i]) - red(pixels[i])) < radius) && (blue(pixels[i]) > brightness) &&
                        red(pixels[i]) > brightness) {
                    pixels[i] = rgb(1, 1, 1); // set the pixel to almost 100% black
                    sum_m += green(pixels[i]) + red(pixels[i]) + blue(pixels[i]);
                    sum_mr += (green(pixels[i]) + red(pixels[i]) + blue(pixels[i])) * i;
                }
                if (sum_m > 5) {
                    COM = sum_mr / sum_m;
                } else {
                    COM = 0;
                }

            }
            bmp.setPixels(pixels, 0, bmp.getWidth(), 0, height, bmp.getWidth(), 1);
            if(COM != 0)
                canvas.drawCircle(COM, height, 5, paint1); // x position, y position, diameter, color
//                if (COM != 0) {
//                    canvas.drawCircle(COM, j, 5, paint1); // x position, y position, diameter, color
//                }

        }


        // write the pos as text
        canvas.drawText("COM = " + COM + " " + bmp.getWidth() + " "  + bmp.getHeight(), 10, 200, paint1);
        c.drawBitmap(bmp, 0, 0, null);
        mSurfaceHolder.unlockCanvasAndPost(c);

        // calculate the FPS to see how fast the code is running
        long nowtime = System.currentTimeMillis();
        long diff = nowtime - prevtime;
        mTextView.setText("FPS " + 1000 / diff);
        prevtime = nowtime;

        //Control calculations to send to motors
        if(carRunning)
        {
            double error = COM - 400;
            servoAngle = 75;
            if(COM != 0) {
                if (error > 0) {

                    leftWheelSpeed = maxSpeed;
                    rightWheelSpeed = maxSpeed - (error / 400) * KP;
                    if (rightWheelSpeed < 0)
                        rightWheelSpeed = 0;
                    servoAngle = 0;
                } else if (error < 0) {
                    leftWheelSpeed = maxSpeed + (error / 400) * KP;
                    rightWheelSpeed = maxSpeed;
                    if (leftWheelSpeed < 0)
                        leftWheelSpeed = 0;
                    servoAngle = 180;
                }

                if (error > -radius * KP && error < radius * KP)
                    servoAngle = 75;


                String sendString = String.valueOf(leftWheelSpeed) + " " + String.valueOf(rightWheelSpeed) + " " + String.valueOf(servoAngle) + '\n';
                try {
                    sPort.write(sendString.getBytes(), 10); // 10 is the timeout
                } catch (IOException e) {
                }
            }

        }
        else
        {
            String sendString = "0 0 0\n";
            try {
                sPort.write(sendString.getBytes(), 10); // 10 is the timeout
            } catch (IOException e) { }
        }
    }

    private void setMyControlListener() {
        myRadius.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            @Override
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                radius = progress;
                radiusText.setText("Radius: " + radius);
            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {

            }

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {

            }
        });

        brightBar.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            @Override
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                brightness = progress;
            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {

            }

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {

            }
        });

        speedBar.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            @Override
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                maxSpeed = progress/100.0;
                maxSpeedText.setText("Max Speed: " + maxSpeed);
            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {

            }

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {

            }
        });

        KPBar.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            @Override
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                KP = progress/100.0;
                KPText.setText("KP: " + KP);
            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {

            }

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {

            }
        });
    }

    private void setMyButtonListener()
    {
        stopButton.setOnClickListener(new View.OnClickListener(){
            @Override
            public void onClick(View v) {
                carRunning = false;
                leftWheelSpeed = maxSpeed;
                rightWheelSpeed = maxSpeed;
                servoAngle = 75;
            }
        });


        startButton.setOnClickListener(new View.OnClickListener(){
            @Override
            public void onClick(View v) {
                carRunning = true;
            }
        });

    }

    private final SerialInputOutputManager.Listener mListener =
            new SerialInputOutputManager.Listener() {
                @Override
                public void onRunError(Exception e) {

                }

                @Override
                public void onNewData(final byte[] data) {
                    MainActivity.this.runOnUiThread(new Runnable() {
                        @Override
                        public void run() {
                            MainActivity.this.updateReceivedData(data);
                        }
                    });
                }
            };

    @Override
    protected void onPause() {
        super.onPause();
        stopIoManager();
        if (sPort != null) {
            try {
                sPort.close();
            } catch (IOException e) {
            }
            sPort = null;
        }
        finish();
    }

    @Override
    protected void onResume() {
        super.onResume();

        ProbeTable customTable = new ProbeTable();
        customTable.addProduct(0x04D8, 0x000A, CdcAcmSerialDriver.class);
        UsbSerialProber prober = new UsbSerialProber(customTable);

        final List<UsbSerialDriver> availableDrivers = prober.findAllDrivers(manager);

        if (availableDrivers.isEmpty()) {
            //check
            return;
        }

        UsbSerialDriver driver = availableDrivers.get(0);
        sPort = driver.getPorts().get(0);

        if (sPort == null) {
            //check
        } else {
            final UsbManager usbManager = (UsbManager) getSystemService(Context.USB_SERVICE);
            UsbDeviceConnection connection = usbManager.openDevice(driver.getDevice());
            if (connection == null) {
                //check
                PendingIntent pi = PendingIntent.getBroadcast(this, 0, new Intent("com.android.example.USB_PERMISSION"), 0);
                usbManager.requestPermission(driver.getDevice(), pi);
                return;
            }

            try {
                sPort.open(connection);
                sPort.setParameters(9600, 8, UsbSerialPort.STOPBITS_1, UsbSerialPort.PARITY_NONE);

            } catch (IOException e) {
                //check
                try {
                    sPort.close();
                } catch (IOException e1) {
                }
                sPort = null;
                return;
            }
        }
        onDeviceStateChange();
    }

    private void stopIoManager() {
        if (mSerialIoManager != null) {
            mSerialIoManager.stop();
            mSerialIoManager = null;
        }
    }

    private void startIoManager() {
        if (sPort != null) {
            mSerialIoManager = new SerialInputOutputManager(sPort, mListener);
            mExecutor.submit(mSerialIoManager);
        }
    }

    private void onDeviceStateChange() {
        stopIoManager();
        startIoManager();
    }

    private void updateReceivedData(byte[] data) {
        //do something with received data

        //for displaying:
        String rxString = null;
        try {
            rxString = new String(data, "UTF-8"); // put the data you got into a string
            wheelSpeedText.setText(rxString);
        } catch (UnsupportedEncodingException e) {
            e.printStackTrace();
        }
    }

}