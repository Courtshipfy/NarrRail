class StoryStorage {
  constructor() {
    this.prefix = 'narrrail_';
  }

  save(storyId, data) {
    const key = this.prefix + storyId;
    const json = JSON.stringify({
      ...data,
      lastModified: new Date().toISOString()
    });
    localStorage.setItem(key, json);
  }

  load(storyId) {
    const key = this.prefix + storyId;
    const json = localStorage.getItem(key);
    return json ? JSON.parse(json) : null;
  }

  list() {
    const stories = [];
    for (let i = 0; i < localStorage.length; i++) {
      const key = localStorage.key(i);
      if (key.startsWith(this.prefix)) {
        const storyId = key.replace(this.prefix, '');
        const data = this.load(storyId);
        stories.push({
          storyId,
          lastModified: data.lastModified,
          nodeCount: data.nodes?.length || 0,
          edgeCount: data.edges?.length || 0
        });
      }
    }
    return stories.sort((a, b) =>
      new Date(b.lastModified) - new Date(a.lastModified)
    );
  }

  delete(storyId) {
    const key = this.prefix + storyId;
    localStorage.removeItem(key);
  }

  // 自动保存
  setupAutoSave(storyId, getDataFn, interval = 30000) {
    return setInterval(() => {
      const data = getDataFn();
      this.save(storyId, data);
      console.log('自动保存成功:', new Date().toLocaleTimeString());
    }, interval);
  }
}

export default new StoryStorage();
