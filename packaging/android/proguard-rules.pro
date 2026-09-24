# Qt invokes parts of its Java bridge from native code, so keep the complete
# bridge while allowing R8 to remove AndroidX code that TinyBloom never calls.
-keep class org.qtproject.qt.android.** { *; }
-keep class org.qtproject.qt.android.bindings.** { *; }
-keepclasseswithmembers,includedescriptorclasses class * {
    native <methods>;
}

# Declared in the Qt Android manifest and used by Android for file sharing.
-keep class androidx.core.content.FileProvider { *; }

-keepattributes *Annotation*,InnerClasses,EnclosingMethod
