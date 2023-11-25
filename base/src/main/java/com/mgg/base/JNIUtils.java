// Copyright 2014 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package com.mgg.base;
import androidx.annotation.Keep;

import java.util.Map;

/**
 * This class provides JNI-related methods to the native library.
 */
@Keep
public class JNIUtils {
    private static final String TAG = "JNIUtils";
    private static ClassLoader sJniClassLoader;

    /**
     * Returns a ClassLoader which can load Java classes from the specified split.
     *
     * @param splitName Name of the split, or empty string for the base split.
     */
    private static ClassLoader getSplitClassLoader(String splitName) {
        return sJniClassLoader != null ? sJniClassLoader : JNIUtils.class.getClassLoader();
    }

    /**
     * Sets the ClassLoader to be used for loading Java classes from native.
     *
     * @param classLoader the ClassLoader to use.
     */
    public static void setClassLoader(ClassLoader classLoader) {
        sJniClassLoader = classLoader;
    }

    /**
     * Helper to convert from java maps to two arrays for JNI.
     */
    public static <K, V> void splitMap(Map<K, V> map, K[] outKeys, V[] outValues) {
        assert map.size() == outKeys.length;
        assert outValues.length == outKeys.length;

        int i = 0;
        for (Map.Entry<K, V> entry : map.entrySet()) {
            outKeys[i] = entry.getKey();
            outValues[i] = entry.getValue();
            i++;
        }
    }
}
