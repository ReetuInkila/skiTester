import React from 'react';
import { Text, View } from 'react-native';

export default function HomeScreen({ pairs, rounds }) {
  return (
    <View>
      <Text>Pairs: {pairs}</Text>
      <Text>Rounds: {rounds}</Text>
    </View>
  );
}
