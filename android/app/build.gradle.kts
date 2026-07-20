plugins {
    id("com.android.application")
}

android {
    namespace = "com.kisakcod.android"
    compileSdk = 37

    defaultConfig {
        applicationId = "com.kisakcod.android"
        minSdk = 26
        targetSdk = 37
        versionCode = 1
        versionName = "0.1.0"

        externalNativeBuild {
            cmake {
                cppFlags += listOf("-std=c++20", "-Wall", "-Wextra", "-Werror")
                arguments += listOf("-DANDROID_STL=c++_static")
            }
        }
    }

    ndkVersion = "27.0.12077973"

    externalNativeBuild {
        cmake {
            path = file("src/main/cpp/CMakeLists.txt")
            version = "3.22.1"
        }
    }
}
