package com.example.keyboard

import android.content.SharedPreferences
import android.inputmethodservice.InputMethodService
import android.inputmethodservice.Keyboard
import android.inputmethodservice.KeyboardView
import android.os.Build
import android.preference.PreferenceManager
import android.util.Log
import android.view.KeyEvent
import android.view.View
import android.view.ViewGroup
import android.view.WindowManager
import android.view.inputmethod.EditorInfo
import android.view.inputmethod.InputConnection

class MyKeyboardService : InputMethodService(), KeyboardView.OnKeyboardActionListener {

    private var keyboardView: KeyboardView? = null
    private var currentKeyboard: Keyboard? = null
    private var currentLanguage = "en"
    private var currentMode = "letters" // letters, symbols, emoji
    private var capsLock = false
    private lateinit var prefs: SharedPreferences

    companion object {
        const val PREF_KEY_LANGUAGE = "keyboard_language"
        const val PREF_KEY_SHOW_POPUP = "keyboard_show_popup"

        // Специальные коды клавиш
        const val KEYCODE_LANG_SWITCH = -2
        const val KEYCODE_EMOJI = -3
        const val KEYCODE_SYMBOLS = -4
        const val KEYCODE_BACK_TO_LETTERS = -6

        private const val TAG = "MyKeyboard"
    }

    override fun onCreate() {
        super.onCreate()
        prefs = PreferenceManager.getDefaultSharedPreferences(this)
        currentLanguage = prefs.getString(PREF_KEY_LANGUAGE, "en") ?: "en"
        Log.d(TAG, "Keyboard service created")
    }

    override fun onCreateInputView(): View {
        keyboardView = layoutInflater.inflate(R.layout.keyboard_view, null) as KeyboardView
        keyboardView?.setOnKeyboardActionListener(this)

        // Загружаем настройку показа всплывающих подсказок
        val showPopup = prefs.getBoolean(PREF_KEY_SHOW_POPUP, true)
        keyboardView?.isPreviewEnabled = showPopup

        loadKeyboard(currentLanguage, currentMode)

        return keyboardView!!
    }

    override fun onStartInputView(info: EditorInfo, restarting: Boolean) {
        super.onStartInputView(info, restarting)
        Log.d(TAG, "Start input view")

        // Возвращаемся к буквам при новом вводе
        if (!restarting) {
            currentMode = "letters"
            loadKeyboard(currentLanguage, currentMode)
        }

        // Поднимаем клавиатуру ВЫШЕ системных кнопок
        moveKeyboardAboveNavBar()

        capsLock = false
        currentKeyboard?.setShifted(false)
        keyboardView?.invalidateAllKeys()
    }

    override fun onWindowShown() {
        super.onWindowShown()
        // При каждом показе окна поднимаем клавиатуру
        moveKeyboardAboveNavBar()
    }

    private fun moveKeyboardAboveNavBar() {
        val view = keyboardView ?: return
        val window = window?.window ?: return

        // Получаем высоту навигационной панели
        val navBarHeight = getNavigationBarHeight()
        Log.d(TAG, "Navigation bar height: $navBarHeight px")

        // Устанавливаем отступ снизу равный высоте навигационной панели
        view.setPadding(0, 0, 0, navBarHeight)
        view.requestLayout()

        // Для Android 11+ также настраиваем
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.R) {
            window.setDecorFitsSystemWindows(false)
        } else {
            @Suppress("DEPRECATION")
            window.decorView.systemUiVisibility = (
                View.SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION or
                View.SYSTEM_UI_FLAG_LAYOUT_STABLE
            )
        }

        Log.d(TAG, "Keyboard moved above nav bar with padding: $navBarHeight px")
    }

    private fun getNavigationBarHeight(): Int {
        val resourceId = resources.getIdentifier("navigation_bar_height", "dimen", "android")
        return if (resourceId > 0) {
            resources.getDimensionPixelSize(resourceId)
        } else {
            // Если не удалось получить системную высоту, используем значение по умолчанию
            (48 * resources.displayMetrics.density).toInt()
        }
    }

    private fun loadKeyboard(language: String, mode: String) {
        currentKeyboard = when (mode) {
            "symbols" -> Keyboard(this, R.xml.keyboard_layout_symbols)
            "emoji" -> Keyboard(this, R.xml.keyboard_layout_emoji)
            else -> {
                when (language) {
                    "ru" -> Keyboard(this, R.xml.keyboard_layout_ru)
                    else -> Keyboard(this, R.xml.keyboard_layout_en)
                }
            }
        }
        keyboardView?.keyboard = currentKeyboard
        keyboardView?.invalidateAllKeys()
        Log.d(TAG, "Keyboard loaded: language=$language, mode=$mode")
    }

    override fun onKey(primaryCode: Int, keyCodes: IntArray?) {
        val inputConnection = currentInputConnection
        Log.d(TAG, "Key pressed: $primaryCode")

        // ВАЖНО: проверяем inputConnection, но для специальных клавиш он может быть null
        when (primaryCode) {
            Keyboard.KEYCODE_DELETE -> { // -5
                inputConnection?.deleteSurroundingText(1, 0) ?: return
            }

            Keyboard.KEYCODE_SHIFT -> { // -1
                capsLock = !capsLock
                currentKeyboard?.setShifted(capsLock)
                keyboardView?.invalidateAllKeys()
                return
            }

            10 -> { // Enter
                inputConnection?.sendKeyEvent(KeyEvent(KeyEvent.ACTION_DOWN, KeyEvent.KEYCODE_ENTER))
                inputConnection?.sendKeyEvent(KeyEvent(KeyEvent.ACTION_UP, KeyEvent.KEYCODE_ENTER))
                    ?: return
            }

            32 -> { // Space
                inputConnection?.commitText(" ", 1) ?: return
            }

            46 -> { // Точка
                inputConnection?.commitText(".", 1) ?: return
            }

            KEYCODE_LANG_SWITCH -> { // -2
                if (currentMode == "letters") {
                    currentLanguage = if (currentLanguage == "en") "ru" else "en"
                } else {
                    currentLanguage = if (currentLanguage == "en") "ru" else "en"
                }
                // Просто обновляем язык, режим оставляем тот же
                loadKeyboard(currentLanguage, currentMode)
                capsLock = false
                currentKeyboard?.setShifted(false)
                return
            }

            KEYCODE_EMOJI -> { // -3
                currentMode = "emoji"
                loadKeyboard(currentLanguage, currentMode)
                capsLock = false
                currentKeyboard?.setShifted(false)
                return
            }

            KEYCODE_SYMBOLS -> { // -4
                currentMode = "symbols"
                loadKeyboard(currentLanguage, currentMode)
                capsLock = false
                currentKeyboard?.setShifted(false)
                return
            }

            KEYCODE_BACK_TO_LETTERS -> { // -6
                currentMode = "letters"
                loadKeyboard(currentLanguage, currentMode)
                capsLock = false
                currentKeyboard?.setShifted(false)
                return
            }

            else -> {
                if (inputConnection == null) return

                // Обычные символы
                val char = primaryCode.toChar()

                // Проверяем, является ли это emoji (коды больше 1000)
                if (primaryCode > 1000) {
                    // Это emoji
                    inputConnection.commitText(String(Character.toChars(primaryCode)), 1)
                } else {
                    // Обычный символ
                    val textToCommit = if (capsLock && char.isLowerCase()) {
                        char.uppercaseChar().toString()
                    } else {
                        char.toString()
                    }
                    inputConnection.commitText(textToCommit, 1)

                    if (capsLock) {
                        capsLock = false
                        currentKeyboard?.setShifted(false)
                        keyboardView?.invalidateAllKeys()
                    }
                }
            }
        }
    }

    override fun onPress(primaryCode: Int) {}
    override fun onRelease(primaryCode: Int) {}
    override fun onText(text: CharSequence?) {
        currentInputConnection?.commitText(text, 1)
    }
    override fun swipeLeft() {}
    override fun swipeRight() {}
    override fun swipeDown() {}
    override fun swipeUp() {}
}