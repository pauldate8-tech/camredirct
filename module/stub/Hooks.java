package com.camtest.stub;
import android.hardware.Camera;
import android.hardware.camera2.CameraManager;
import android.hardware.camera2.CameraDevice;
import android.os.Handler;
import java.util.concurrent.Executor;

public class Hooks {
    public static Method backupOpen0, backupOpen1, backupOpen2, backupOpen2Exec;

    static native int    remapCamera1(int requested);
    static native String remapCamera2(CameraManager mgr, String requested);

    public static Camera openCamera1Replacement(int id) throws Throwable {
        return (Camera) backupOpen1.invoke(null, remapCamera1(id));
    }
    public static Camera openCamera0Replacement() throws Throwable {
        int id = remapCamera1(0);
        if (backupOpen1 != null) return (Camera) backupOpen1.invoke(null, id);
        return (Camera) backupOpen0.invoke(null);
    }
    public static void openCamera2Replacement(CameraManager self, String id,
            CameraDevice.StateCallback cb, Handler h) throws Throwable {
        backupOpen2.invoke(self, remapCamera2(self, id), cb, h);
    }
    public static void openCamera2ExecReplacement(CameraManager self, String id,
            Executor e, CameraDevice.StateCallback cb) throws Throwable {
        backupOpen2Exec.invoke(self, remapCamera2(self, id), e, cb);
    }
}
