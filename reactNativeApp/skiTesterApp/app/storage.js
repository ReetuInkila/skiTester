import AsyncStorage from '@react-native-async-storage/async-storage';

const STORAGE_KEY = 'measurementData';

// Tallenna data
export const saveDataToStorage = async (data) => {
  try {
    await AsyncStorage.setItem(STORAGE_KEY, JSON.stringify(data));
  } catch (error) {
    console.error('Failed to save data:', error);
  }
};

// Lataa data
export const loadDataFromStorage = async () => {
  try {
    const jsonData = await AsyncStorage.getItem(STORAGE_KEY);
    return jsonData != null ? JSON.parse(jsonData) : null;
  } catch (error) {
    console.error('Failed to load data:', error);
    return null;
  }
};

// Tyhjennä data
export const clearStorage = async () => {
  try {
    await AsyncStorage.removeItem(STORAGE_KEY);
  } catch (error) {
    console.error('Failed to clear data:', error);
  }
};

export default {
  saveDataToStorage,
  loadDataFromStorage,
  clearStorage,
};
