package com.poc.p_couds

import android.content.Context
import android.content.SharedPreferences
import android.os.Bundle
import android.util.Log
import androidx.activity.compose.setContent
import androidx.activity.enableEdgeToEdge
import androidx.appcompat.app.AppCompatActivity
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Spacer
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.padding
//noinspection UsingMaterialAndMaterial3Libraries
import androidx.compose.material.Scaffold
//noinspection UsingMaterialAndMaterial3Libraries
import androidx.compose.material.Text
//noinspection UsingMaterialAndMaterial3Libraries
import androidx.compose.material.TextButton
//noinspection UsingMaterialAndMaterial3Libraries
import androidx.compose.material.TopAppBar
import androidx.compose.runtime.Composable
import androidx.compose.ui.Modifier
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.unit.dp
import androidx.navigation.NavController
import androidx.navigation.NavHostController
import androidx.navigation.compose.NavHost
import androidx.navigation.compose.composable
import androidx.navigation.compose.rememberNavController
import com.poc.p_couds.ui.theme.CarsDataTheme
import com.google.accompanist.systemuicontroller.rememberSystemUiController
import androidx.compose.foundation.layout.systemBarsPadding
import androidx.compose.foundation.layout.width
//noinspection UsingMaterialAndMaterial3Libraries
import androidx.compose.material.DropdownMenu
//noinspection UsingMaterialAndMaterial3Libraries
import androidx.compose.material.Icon
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.filled.AccountBox
import androidx.compose.material.icons.filled.AccountCircle
import androidx.compose.material.icons.filled.Build
import androidx.compose.material.icons.filled.Home
import androidx.compose.material.icons.filled.Person
import androidx.compose.material3.DropdownMenuItem
import androidx.compose.runtime.DisposableEffect
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.setValue
import androidx.compose.ui.platform.LocalContext
import androidx.compose.ui.viewinterop.AndroidView
import androidx.fragment.app.Fragment
import androidx.fragment.app.FragmentContainerView
import androidx.fragment.app.commit

class MainActivity : AppCompatActivity() {
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        enableEdgeToEdge()
        setContent {
            CarsDataTheme {
                MainScreen()
            }
        }
    }

    @Composable
    fun ShowFragmentHome() {
        FragmentContainer(Modifier.fillMaxSize(), HomeFragment())
    }

    @Composable
    fun ShowFragmentLogin() {
        FragmentContainer(Modifier.fillMaxSize(), LogInFragment())
    }

    @Composable
    fun ShowFragmentSignup() {
        FragmentContainer(Modifier.fillMaxSize(), SignUpFragment())
    }

    @Composable
    fun ShowFragmentServices() {
        FragmentContainer(Modifier.fillMaxSize(), AccessibilityFragment())
    }

    @Composable
    fun FragmentContainer(modifier: Modifier, fragment: Fragment) {
        AndroidView(
            modifier = modifier,
            factory = { context ->
                FragmentContainerView(context).apply {
                    id = R.id.fragment_container_view // Make sure this ID is defined in ids.xml
                }
            },
            update = { fragmentContainerView ->
                supportFragmentManager.commit {
                    setReorderingAllowed(true)
                    setCustomAnimations(R.anim.fade_in_custom, R.anim.fade_out_custom)
                    replace(fragmentContainerView.id, fragment)
                }
            }
        )
    }


    @Composable
    fun AppNavigator(navController: NavHostController) {
        NavHost(
            navController = navController,
            startDestination = "home"
        ) {
            composable("home") { ShowFragmentHome() }
            composable("login") { ShowFragmentLogin() }
            composable("signup") { ShowFragmentSignup() }
            composable("services") { ShowFragmentServices() }
        }
    }

    @Composable
    fun TopAppBarWithMenu(navController: NavController) {
        val context = LocalContext.current
        val sharedPreferences = context.getSharedPreferences("Preferences", Context.MODE_PRIVATE)
        var isLoggedIn by remember {
            mutableStateOf(sharedPreferences.getBoolean("isLoggedIn", false))
        }
        val email = sharedPreferences.getString("loggedInUser", "")

        // Extract the first name
        val firstName = email?.substringBefore(".")
            ?.substringBefore("_")
            ?.replaceFirstChar { if (it.isLowerCase()) it.titlecase() else it.toString() }
            ?: "User"

        var expanded by remember { mutableStateOf(false) }

        // LaunchEffect to listen for changes in shared preferences
        DisposableEffect(Unit) {
            val listener = SharedPreferences.OnSharedPreferenceChangeListener { _, key ->
                if (key == "isLoggedIn") {
                    isLoggedIn = sharedPreferences.getBoolean("isLoggedIn", false)
                }
            }
            sharedPreferences.registerOnSharedPreferenceChangeListener(listener)

            // Cleanup the listener when the composable is disposed
            onDispose {
                sharedPreferences.unregisterOnSharedPreferenceChangeListener(listener)
            }
        }

        TopAppBar(
            title = {
                Column {
                    Text("Drive")
                    Text("Insight")}},
            backgroundColor = Color.LightGray,
            actions = {
                TextButton(onClick = { navController.navigate("home") }) {
                    Icon(Icons.Filled.Home, contentDescription = "Home", tint = Color.White)
                    Spacer(modifier = Modifier.width(8.dp))
                    Text("Home", color = Color.White)
                }

                if (isLoggedIn) {
                    TextButton(onClick = { navController.navigate("services") }) {
                        Icon(Icons.Filled.Build, contentDescription = "Services", tint = Color.White)
                        Spacer(modifier = Modifier.width(8.dp))
                        Text("Services", color = Color.White)
                    }
                    Box(
                        modifier = Modifier
                            .padding(end = 16.dp)  // Optional: add some padding
                    ) {
                        TextButton(onClick = { expanded = !expanded }) {
                            Icon(Icons.Filled.AccountCircle, contentDescription = "User Profile", tint = Color.White)
                            Spacer(modifier = Modifier.width(8.dp))
                            Text(text = firstName,
                                color = Color.White)
                        }

                        // Dropdown menu
                        DropdownMenu(
                            expanded = expanded,
                            onDismissRequest = { expanded = false }
                        ) {
                            DropdownMenuItem(
                                text = {Text("Sign out")},
                                onClick = {
                                expanded = false
                                // Handle logout
                                sharedPreferences.edit().clear().apply()  // Clear login info
                                isLoggedIn = false
                                navController.navigate("login")  // Navigate to login
                            })
                        }
                    }
                } else {
                    TextButton(onClick = { navController.navigate("login") }) {
                        Icon(Icons.Filled.Person, contentDescription = "Log in", tint = Color.White)
                        Spacer(modifier = Modifier.width(8.dp))
                        Text("Log in", color = Color.White)
                    }
                    TextButton(onClick = { navController.navigate("signup") }) {
                        Icon(
                            Icons.Filled.AccountBox,
                            contentDescription = "Sign up",
                            tint = Color.White
                        )
                        Spacer(modifier = Modifier.width(8.dp))
                        Text("Sign Up", color = Color.White)
                    }
                }
            })
    }

    @Composable
    fun MainScreen() {
        val navController = rememberNavController()
        val systemUiController = rememberSystemUiController()
        systemUiController.setSystemBarsColor(color = Color.Gray)  // Make system bars transparent

        Scaffold(
            topBar = {
                TopAppBarWithMenu(navController)
            },
            modifier = Modifier.systemBarsPadding()
        ){  paddingValues  ->
            Box(modifier = Modifier
                .fillMaxSize()
                .padding(paddingValues)) {
                AppNavigator(navController)
            }
        }

    }
}



