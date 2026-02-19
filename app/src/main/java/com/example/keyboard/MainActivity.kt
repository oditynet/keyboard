package com.example.keyboard

import android.content.Context
import android.content.Intent
import android.os.Bundle
import android.provider.Settings
import android.view.View
import android.view.inputmethod.InputMethodManager
import android.widget.Button
import androidx.appcompat.app.AlertDialog
import androidx.appcompat.app.AppCompatActivity

class MainActivity : AppCompatActivity() {

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)

        val btnSettings = findViewById<Button>(R.id.btnSettings)
        val btnTestKeyboard = findViewById<Button>(R.id.btnTestKeyboard)

        btnSettings.setOnClickListener {
            // Открываем настройки клавиатуры
            val intent = Intent(this, SettingsActivity::class.java)
            startActivity(intent)
        }

        btnTestKeyboard.setOnClickListener {
            // Открываем системные настройки клавиатуры
            startActivity(Intent(Settings.ACTION_INPUT_METHOD_SETTINGS))
        }

        // Проверяем, включена ли наша клавиатура в системе
        checkKeyboardEnabled()
    }

    private fun checkKeyboardEnabled() {
        val imm = getSystemService(Context.INPUT_METHOD_SERVICE) as InputMethodManager
        val enabledServices = imm.enabledInputMethodList

        var ourKeyboardEnabled = false
        for (service in enabledServices) {
            if (service.packageName == packageName) {
                ourKeyboardEnabled = true
                break
            }
        }

        if (!ourKeyboardEnabled) {
            showEnableKeyboardDialog()
        }
    }

    private fun showEnableKeyboardDialog() {
        AlertDialog.Builder(this)
            .setTitle("Включите клавиатуру")
            .setMessage("Чтобы использовать клавиатуру, её нужно включить в настройках:\n\n" +
                    "1. Нажмите 'Открыть настройки'\n" +
                    "2. Выберите 'Виртуальная клавиатура'\n" +
                    "3. Включите переключатель рядом с 'My Keyboard'")
            .setPositiveButton("Открыть настройки") { _, _ ->
                startActivity(Intent(Settings.ACTION_INPUT_METHOD_SETTINGS))
            }
            .setNegativeButton("Закрыть", null)
            .show()
    }
}