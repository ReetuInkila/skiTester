import { configureStore, createSlice } from '@reduxjs/toolkit';

const initialState = {
  data: {
    order: [],
    names: [],
    temperature: 0,
    snowQuality: 'unknown',
    baseHardness: 'unknown',
    pairs: 5,
    rounds: 5,
    results: [],
  },
  serverState: 'Yhdistetään...',
};

const appSlice = createSlice({
  name: 'app',
  initialState,
  reducers: {
    setData(state, action) {
      state.data = { ...state.data, ...action.payload };
    },
    addResult(state, action) {
      state.data.results.push(action.payload);
    },
    setServerState(state, action) {
      state.serverState = action.payload;
    },
    setOrder(state, action) {
      state.data.order = action.payload; 
    },
    clearData(state) {
      state.data = { ...initialState.data };
    },
  },
});

export const { setData, addResult, setServerState, setOrder, clearData } = appSlice.actions;

const store = configureStore({
  reducer: {
    app: appSlice.reducer,
  },
});

export default store;