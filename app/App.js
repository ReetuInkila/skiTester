import React, { useState } from 'react';
import { NavigationContainer } from '@react-navigation/native';
import { createStackNavigator } from '@react-navigation/stack';
import { StyleSheet, Image, TouchableOpacity } from 'react-native';
import StartScreen from './StartScreen';
import HomeScreen from './Home';
import SettingsScreen from './Settings';
import ResultScreen from './ResultScreen';
import Ionicons from 'react-native-vector-icons/Ionicons';

const Stack = createStackNavigator();

export default function App() {
  const [pairs, setPairs] = useState(5);
  const [rounds, setRounds] = useState(5);
  const [data, setData] = useState({ pairs: pairs, rounds: rounds, results: [] });

  return (
    <NavigationContainer>
      <Stack.Navigator initialRouteName="Start">
        {/* Start Screen */}
        <Stack.Screen
          name="Start"
          component={StartScreen}
          options={{
            title: 'Aloitus',
            headerTitle: () => <Image source={require('./assets/logo.png')} style={styles.logo} />,
          }}
        />

        {/* Home Screen */}
        <Stack.Screen
          name="Home"
          options={({ navigation }) => ({
            headerTitle: () => <Image source={require('./assets/logo.png')} style={styles.logo} />,
            headerRight: () => (
              <TouchableOpacity
                onPress={() => navigation.navigate('Settings')}
                style={styles.headerButton}
              >
                <Ionicons name="settings-outline" size={24} color="black" />
              </TouchableOpacity>
            ),
          })}
        >
          {(props) => (
            <HomeScreen
              {...props}
              data={data}
              setData={setData}
              pairs={pairs}
              rounds={rounds}
            />
          )}
        </Stack.Screen>

        {/* Settings Screen */}
        <Stack.Screen
          name="Settings"
          options={{
            title: 'Asetukset',
            headerTitle: () => <Image source={require('./assets/logo.png')} style={styles.logo} />,
          }}
        >
          {(props) => (
            <SettingsScreen
              {...props}
              pairs={pairs}
              setPairs={setPairs}
              rounds={rounds}
              setRounds={setRounds}
            />
          )}
        </Stack.Screen>

        {/* Results Screen */}
        <Stack.Screen
          name="Results"
          options={{
            title: 'Tulokset',
            headerTitle: () => <Image source={require('./assets/logo.png')} style={styles.logo} />,
          }}
        >
          {(props) => (
            <ResultScreen
              {...props}
              results={data.results}
            />
          )}
        </Stack.Screen>
      </Stack.Navigator>
    </NavigationContainer>
  );
}

const styles = StyleSheet.create({
  logo: {
    height: 40, // Adjust logo height
    resizeMode: 'contain', // Ensure the logo fits nicely
  },
  headerButton: {
  },
});
