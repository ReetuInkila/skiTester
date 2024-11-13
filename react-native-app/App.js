import React, { useState } from 'react';
import { NavigationContainer } from '@react-navigation/native';
import { createStackNavigator } from '@react-navigation/stack';
import { StyleSheet } from 'react-native';
import StartScreen from './StartScreen';
import HomeScreen from './Home';
import SettingsScreen from './Settings';
import ResultScreen from './ResultScreen';

const Stack = createStackNavigator();

export default function App() {
  const [pairs, setPairs] = useState(5);
  const [rounds, setRounds] = useState(5);
  const [data, setData] = useState({ pairs: pairs, rounds: rounds, results: [] });

  return (
    <NavigationContainer>
      <Stack.Navigator initialRouteName="Start">
        <Stack.Screen name="Start" component={StartScreen} options={{ title: 'Aloitus' }} />
        <Stack.Screen name="Home" options={{title: 'Mittaus'}}>
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
        <Stack.Screen name="Results" options={{ title: 'Tulokset' }}>
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
  headerButton: {
    marginRight: 15,
  },
});
