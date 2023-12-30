package com.mgg.test

import androidx.appcompat.app.AppCompatActivity
import android.os.Bundle
import com.mgg.base.NativeLib
import com.mgg.test.databinding.ActivityMainBinding

class MainActivity : AppCompatActivity() {

    private lateinit var binding: ActivityMainBinding

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        binding = ActivityMainBinding.inflate(layoutInflater)
        setContentView(binding.root)

        // Example of a call to a native method
        binding.sampleText.text = stringFromJNI()
        binding.sampleText.setOnClickListener {
            binding.sampleText.text = NativeLib().stringFromJNI()
        }
    }

    /**
     * A native method that is implemented by the 'buglyversion' native library,
     * which is packaged with this application.
     */
    external fun stringFromJNI(): String

    companion object {
        // Used to load the 'buglyversion' library on application startup.
        init {
            System.loadLibrary("app")
        }
    }
}