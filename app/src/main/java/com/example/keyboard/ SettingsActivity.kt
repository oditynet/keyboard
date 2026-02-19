package com.example.keyboard

import android.content.SharedPreferences
import android.os.Bundle
import android.preference.PreferenceManager
import android.widget.*
import androidx.appcompat.app.AppCompatActivity

class SettingsActivity : AppCompatActivity() {

    private lateinit var prefs: SharedPreferences
    private lateinit var spinnerLanguage: Spinner
    private lateinit var switchShowPopup: Switch
    private lateinit var btnSave: Button
    private lateinit var btnCancel: Button

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_settings)

        title = "Настройки клавиатуры"

        prefs = PreferenceManager.getDefaultSharedPreferences(this)

        // Инициализация
        spinnerLanguage = findViewById(R.id.spinnerLanguage)
        switchShowPopup = findViewById(R.id.switchShowPopup)
        btnSave = findViewById(R.id.btnSave)
        btnCancel = findViewById(R.id.btnCancel)

        setupLanguageSpinner()
        setupShowPopupSwitch()
        loadCurrentSettings()

        btnSave.setOnClickListener {
            saveSettings()
            Toast.makeText(this, "Настройки сохранены", Toast.LENGTH_SHORT).show()
            finish()
        }

        btnCancel.setOnClickListener {
            finish()
        }
    }

    private fun setupLanguageSpinner() {
        val languages = arrayOf("Английская (EN)", "Русская (RU)")
        val adapter = ArrayAdapter(this, android.R.layout.simple_spinner_item, languages)
        adapter.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item)
        spinnerLanguage.adapter = adapter
    }

    private fun setupShowPopupSwitch() {
        switchShowPopup.text = "Показывать увеличенную букву при нажатии"
    }

    private fun loadCurrentSettings() {
        // Загружаем язык
        val savedLanguage = prefs.getString(MyKeyboardService.PREF_KEY_LANGUAGE, "en")
        spinnerLanguage.setSelection(if (savedLanguage == "ru") 1 else 0)

        // Загружаем настройку показа всплывающих подсказок
        val showPopup = prefs.getBoolean(MyKeyboardService.PREF_KEY_SHOW_POPUP, true)
        switchShowPopup.isChecked = showPopup
    }

    private fun saveSettings() {
        val editor = prefs.edit()

        // Сохраняем язык
        val language = if (spinnerLanguage.selectedItemPosition == 1) "ru" else "en"
        editor.putString(MyKeyboardService.PREF_KEY_LANGUAGE, language)

        // Сохраняем показ всплывающих подсказок
        editor.putBoolean(MyKeyboardService.PREF_KEY_SHOW_POPUP, switchShowPopup.isChecked)

        editor.apply()
    }
}