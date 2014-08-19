package com.mediatek.aaltool;

import android.os.Bundle;
import android.provider.Settings;
import android.app.Activity;
import android.util.Log;
import android.view.Menu;
import android.view.View;
import android.view.Window;
import android.view.WindowManager.LayoutParams;
import android.widget.TextView;
import android.widget.SeekBar;
import android.widget.SeekBar.OnSeekBarChangeListener;
import android.widget.AbsoluteLayout;
import android.widget.Button;
import android.content.ContentResolver;
import android.content.SharedPreferences;
import android.content.Context;
import android.hardware.Sensor;
import android.hardware.SensorEvent;
import android.hardware.SensorEventListener;
import android.hardware.SensorManager;
import java.io.FileOutputStream;
import java.io.PrintWriter;
import java.util.Map;

public class AALTuning extends Activity implements OnSeekBarChangeListener, SensorEventListener {
    private static final String TAG = "AALTool";
    private static final String PREFS_NAME = "aal";
    private static final String FILE_NAME = "aal.cfg";
    private SeekBar mLABCBar;
    private SeekBar mCABCBar;
    private SeekBar mDREBar;
    private TextView mLABCText;
    private TextView mCABCText;
    private TextView mDREText;
    private int mLABCLevel = 2;
    private int mCABCLevel = 2;
    private int mDRELevel = 2;
    private SensorManager mSensorManager;
    private Sensor mLightSensor;
    private float mALS = 0;
    private Button mSaveButton;
    private SharedPreferences mPreferences;
    private int mBrightness = 255;
    private int mBrightnessMode = Settings.System.SCREEN_BRIGHTNESS_MODE_MANUAL;
    
    //the content resolver used as a handle to the system's settings  
    private ContentResolver mContentResolver;  
    //a window object, that will store a reference to the current window  
    private Window mWindow;
    
    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.aal_tuning);        
        
        Log.d(TAG, "onCreate...");
        
        
        //get the content resolver  
        mContentResolver = getContentResolver();
        //get the current window  
        mWindow = getWindow();
        
        mSaveButton = (Button)findViewById(R.id.buttonSave);
        mSaveButton.setOnClickListener(new Button.OnClickListener() {
            public void onClick(View v) {
                // Perform action on click
                saveToFile();
            }
        });
        
        mPreferences = getSharedPreferences(PREFS_NAME, 0);
        loadPreference();

        mLABCText = (TextView) this.findViewById(R.id.textLABCLevel); 
        mLABCText.setText("level: " + Integer.toString(mLABCLevel));
        mLABCBar = (SeekBar)findViewById(R.id.seekBarLABC); // make seekbar object
        mLABCBar.setOnSeekBarChangeListener(this); // set seekbar listener.
        mLABCBar.setProgress(mLABCLevel);
        mCABCText = (TextView) this.findViewById(R.id.textCABCLevel); 
        mCABCText.setText("level: " + Integer.toString(mCABCLevel));
        mCABCBar = (SeekBar)findViewById(R.id.seekBarCABC); // make seekbar object
        mCABCBar.setOnSeekBarChangeListener(this); // set seekbar listener.
        mCABCBar.setProgress(mCABCLevel);
        mDREText = (TextView) this.findViewById(R.id.textDRELevel); 
        mDREText.setText("level: " + Integer.toString(mDRELevel));
        mDREBar = (SeekBar)findViewById(R.id.seekBarDRE); // make seekbar object
        mDREBar.setOnSeekBarChangeListener(this); // set seekbar listener.       
        mDREBar.setProgress(mDRELevel);

        mSensorManager = (SensorManager) this.getSystemService(SENSOR_SERVICE);
        mLightSensor = mSensorManager.getDefaultSensor(Sensor.TYPE_LIGHT);
    }

    private void setScreenBrightness() {
        try {
             mBrightness = Settings.System.getInt(mContentResolver, Settings.System.SCREEN_BRIGHTNESS);
        } catch (Exception e) {
            e.printStackTrace();
        }
        nSetBacklight(mBrightness);
    }
    
    private void loadPreference() {
        Map<String,?> keys = mPreferences.getAll();
        for(Map.Entry<String,?> entry : keys.entrySet()) {
            Log.d(TAG, "map values " + entry.getKey() + ": " + entry.getValue().toString());
            int value = Integer.parseInt(entry.getValue().toString());
            if (entry.getKey().equals("LABC"))
                mLABCLevel = value;
            if (entry.getKey().equals("CABC")) 
                mCABCLevel = value;
            if (entry.getKey().equals("DRE")) 
                mDRELevel = value;                
        }
    }
    private void saveToFile() {
        try {
            
            FileOutputStream fos = openFileOutput(FILE_NAME, Context.MODE_PRIVATE);
            PrintWriter pw = new PrintWriter(fos); 
            pw.println("LABC=" + mLABCLevel);
            pw.println("CABC=" + mCABCLevel);
            pw.println("DRE=" + mDRELevel);
            pw.close();
            fos.close();
        } catch(Exception e){
            e.printStackTrace();
        }
    }

    
    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        getMenuInflater().inflate(R.menu.aal_tuning, menu);
        return true;
    }

    @Override
    public void onProgressChanged(SeekBar arg0, int arg1, boolean arg2) {        
        String key = "";
        if (arg0 == mLABCBar) {
            Log.d(TAG, "LABC level = " + arg1);
            if (nSetLABCLevel(arg1)) {
                key = "LABC";
                mLABCLevel = arg1;
                mLABCText.setText("level: " + Integer.toString(mLABCLevel));
            }
        }
        if (arg0 == mCABCBar) {
            Log.d(TAG, "set CABC level = " + arg1);
            if (nSetCABCLevel(arg1)) {
                key = "CABC";
                mCABCLevel = arg1;
                mCABCText.setText("level: " + Integer.toString(mCABCLevel));
            }
        }
        if (arg0 == mDREBar) {
            Log.d(TAG, "set DRE level = " + arg1);
            if (nSetDRELevel(arg1)) {
                key = "DRE";
                mDRELevel = arg1;
                mDREText.setText("level: " + Integer.toString(mDRELevel));
            }
        }
        setScreenBrightness();

        if (key.length() > 0) {
            SharedPreferences.Editor editor = mPreferences.edit();
            editor.putInt(key, arg1);
            editor.commit();
        }

    }

    @Override
    public void onStartTrackingTouch(SeekBar seekBar) {
        // TODO Auto-generated method stub
        
    }

    @Override
    public void onStopTrackingTouch(SeekBar seekBar) {
        // TODO Auto-generated method stub
        
    }
    
    @Override
    protected void onResume() {
        super.onResume();
        Log.d(TAG, "registerListener...");
        if (mLightSensor != null) {
            mSensorManager.registerListener(this, mLightSensor, 
                    SensorManager.SENSOR_DELAY_NORMAL);
        }
    }

    @Override
    protected void onPause() {
        super.onPause();
        Log.d(TAG, "unregisterListener...");
        if (mLightSensor != null) {
            mSensorManager.unregisterListener(this, mLightSensor);
        }
    }
    
    @Override
    public void onAccuracyChanged(Sensor sensor, int accuracy) {
        // TODO Auto-generated method stub
        
    }

    @Override
    public void onSensorChanged(SensorEvent event) {
        // TODO Auto-generated method stub
        mALS = event.values[0];
        if (mLABCLevel > 0) {
            Log.d(TAG, "get sensor data: " + mALS);
        }
    }
    static {
        System.loadLibrary("aaltool_jni");
    }

    private native boolean nSetLABCLevel(int level);
    private native boolean nSetCABCLevel(int level);
    private native boolean nSetDRELevel(int level);
    private native boolean nSetLightSensorValue(int value);
    private native boolean nSetBacklight(int level);
    
}
