import React, { useState } from 'react';
import { TouchableOpacity } from 'react-native';
import { NavigationContainer } from '@react-navigation/native';
import { createStackNavigator } from '@react-navigation/stack';
import { Ionicons } from '@expo/vector-icons';
import HomeScreen from './Home';
import SettingsScreen from './Settings';

const Stack = createStackNavigator();

export default function App() {
  // State management for pairs and rounds
  const [pairs, setPairs] = useState(5);
  const [rounds, setRounds] = useState(5);

  return (
    <NavigationContainer>
      <Stack.Navigator initialRouteName="Home">
        <Stack.Screen
          name="Home"
          options={({ navigation }) => ({
            title: 'Home',
            headerRight: () => (
              <TouchableOpacity onPress={() => navigation.navigate('Settings')} style={{ marginRight: 15 }}>
                <Ionicons name="settings-outline" size={24} color="black" />
              </TouchableOpacity>
            ),
          })}
        >
          {(props) => <HomeScreen {...props} pairs={pairs} rounds={rounds} />}
        </Stack.Screen>
        <Stack.Screen name="Settings">
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
