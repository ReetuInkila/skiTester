import React, { useState } from 'react';
import { NavigationContainer } from '@react-navigation/native';
import { createStackNavigator } from '@react-navigation/stack';
import { TouchableOpacity, StyleSheet } from 'react-native';
import { Ionicons } from '@expo/vector-icons';
import StartScreen from './StartScreen';
import HomeScreen from './Home';
import SettingsScreen from './Settings';

const Stack = createStackNavigator();

export default function App() {
  const [pairs, setPairs] = useState(5);
  const [rounds, setRounds] = useState(5);

  return (
    <NavigationContainer>
      <Stack.Navigator initialRouteName="Start">
        <Stack.Screen name="Start" component={StartScreen} options={{ title: 'Aloitus' }} />
        <Stack.Screen
          name="Home"
          options={({ navigation }) => ({
            title: 'Mittaus',
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
          {(props) => <HomeScreen {...props} pairs={pairs} rounds={rounds} />}
        </Stack.Screen>
        <Stack.Screen name="Settings" options={{ title: 'Asetukset' }}>
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
      </Stack.Navigator>
    </NavigationContainer>
  );
}

const styles = StyleSheet.create({
  headerButton: {
    marginRight: 15,
  },
});
