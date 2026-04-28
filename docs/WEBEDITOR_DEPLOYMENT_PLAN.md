# NarrRail WebEditor 云端认证与存储部署方案

## 1. 文档目标

本方案用于指导 `NarrRail.WebEditor` 从“本地编辑 + localStorage”升级为“云端登录 + 持久化存储 + GitHub 绑定”的可用线上版本，满足以下目标：

- 任意用户可登录并拥有自己的文件库
- 编辑内容可跨设备长期保存
- 可选与 GitHub 账号/仓库绑定，实现导入导出与版本管理
- 后端保持“薄服务”架构，降低运维复杂度与成本

---

## 2. 目标能力范围（MVP）

### 2.1 必做（MVP）
1. GitHub OAuth 登录
2. 用户项目/剧情云端持久化（非本地）
3. 自动保存（前端定时 + 后端幂等写入）
4. 基础版本记录（至少保留最近 N 次快照）
5. 安全隔离（用户只能访问自己的数据）

### 2.2 可选（V1.1+）
1. GitHub 仓库绑定并提交 YAML
2. 从 GitHub 拉取指定文件导入
3. 冲突提示（云端版本号不一致）
4. 分享只读链接（短链 + 签名）

---

## 3. 推荐总体架构

## 3.1 架构原则
- **前端只负责 UI 与编辑状态，不持有敏感密钥**
- **OAuth Secret、GitHub Access Token 仅后端可见**
- **编辑主存储在云数据库，GitHub 同步是附加能力**
- **localStorage 保留为离线缓存，不作为权威数据源**

## 3.2 逻辑组件
1. WebEditor 前端（Vue + Svelte Flow）
2. 轻量后端（Serverless Functions）
3. 云数据库（PostgreSQL / Supabase）
4. GitHub OAuth 与 API（可选同步）
5. 监控与日志（Sentry/平台日志）

## 3.3 推荐技术组合（成本优先）
- 前端托管：Vercel / Cloudflare Pages / Netlify
- 后端函数：Cloudflare Workers（优先）或 Vercel Functions
- 数据库：Supabase Postgres（优先）
- 对象存储（可选）：Supabase Storage / R2
- 观测：Sentry + 平台日志

---

## 4. 身份认证设计（GitHub OAuth）

## 4.1 为什么需要后端
OAuth 的 `client_secret` 不能暴露在前端。  
后端负责：
- 处理 OAuth 回调
- 用 `code` 换 token
- 生成平台自己的 session/JWT
- 按需代理 GitHub API

## 4.2 授权流程
1. 前端点击“GitHub 登录”
2. 跳转后端 `/auth/github/start`
3. 后端重定向 GitHub 授权页（携带 `state`）
4. GitHub 回调 `/auth/github/callback?code=...&state=...`
5. 后端校验 `state`，换取 GitHub token
6. 后端创建/更新本地用户，签发 session/JWT
7. 重定向回前端并进入已登录态

## 4.3 权限建议（最小权限）
- 仅登录：`read:user user:email`
- 需要仓库写入时再请求：`repo`（或更细粒度 GitHub App 方案）

---

## 5. 数据模型设计（建议）

## 5.1 users
- `id` (uuid, pk)
- `github_user_id` (unique)
- `github_login`
- `email`
- `avatar_url`
- `created_at`, `updated_at`

## 5.2 projects
- `id` (uuid, pk)
- `owner_user_id` (fk -> users.id)
- `name`
- `description`
- `created_at`, `updated_at`

## 5.3 stories
- `id` (uuid, pk)
- `project_id` (fk)
- `story_id`（编辑器 meta.storyId）
- `title`
- `content_json`（nodes/edges/meta/variables/presetSpeakers）
- `version`（整数，乐观锁）
- `last_saved_at`

## 5.4 story_versions
- `id` (uuid, pk)
- `story_id` (fk)
- `version`
- `snapshot_json`
- `created_by_user_id`
- `created_at`

## 5.5 github_bindings（可选）
- `id` (uuid, pk)
- `user_id` (fk)
- `repo_owner`
- `repo_name`
- `default_branch`
- `encrypted_access_token`（或 token 引用）
- `created_at`, `updated_at`

---

## 6. API 设计（MVP）

## 6.1 认证
- `GET /auth/github/start`
- `GET /auth/github/callback`
- `POST /auth/logout`
- `GET /auth/me`

## 6.2 项目与故事
- `GET /projects`
- `POST /projects`
- `GET /projects/:projectId/stories`
- `POST /projects/:projectId/stories`
- `GET /stories/:storyId`
- `PUT /stories/:storyId`（带 version，冲突时 409）
- `POST /stories/:storyId/snapshots`

## 6.3 GitHub 同步（可选）
- `POST /github/bindings`
- `POST /github/export/:storyId`
- `POST /github/import`

---

## 7. 前端改造要点（WebEditor）

## 7.1 状态分层
- `editorState`：当前节点/边/变量/元数据
- `authState`：登录态、用户信息
- `cloudState`：项目、故事、云端保存状态
- `syncState`：GitHub 绑定与同步状态

## 7.2 自动保存策略
- 保留现有 localStorage（离线兜底）
- 新增云端自动保存（节流 3~8 秒）
- 保存时携带 `version`
- 409 冲突时提示“覆盖 / 拉取最新 / 另存副本”

## 7.3 UI 状态
- 未登录：仅本地
- 已登录：云端自动保存
- 已绑定仓库：可提交 GitHub

---

## 8. 安全基线（必须）

1. 所有敏感信息放环境变量（不可写入前端）
2. session cookie 使用 `HttpOnly + Secure + SameSite=Lax/Strict`
3. OAuth `state` 强校验，防 CSRF
4. API 鉴权中间件统一验证用户身份
5. 数据访问必须带 owner 过滤（防越权）
6. GitHub token 加密存储（KMS/平台密钥）
7. 基础限流（按 IP + 用户）
8. 审计日志（登录、导出、删除等敏感操作）

---

## 9. 部署步骤（Cloudflare Workers + Supabase 示例）

## 9.1 准备阶段
1. 创建 GitHub OAuth App
   - 回调地址：`https://<api-domain>/auth/github/callback`
2. 创建 Supabase 项目并拿到连接信息
3. 准备前端域名与后端域名
4. 配置环境变量（见 9.4）

## 9.2 数据库初始化
1. 创建上述表结构与索引
2. 创建外键与唯一约束
3. 添加迁移脚本（建议使用 migration 工具）

## 9.3 后端部署
1. 初始化函数项目
2. 实现认证与故事 CRUD API
3. 配置 CORS（仅允许前端域名）
4. 部署到生产环境

## 9.4 环境变量清单
- `GITHUB_CLIENT_ID`
- `GITHUB_CLIENT_SECRET`
- `GITHUB_REDIRECT_URI`
- `JWT_SECRET`（或 session secret）
- `DATABASE_URL`
- `APP_FRONTEND_URL`
- `APP_API_URL`
- `ENCRYPTION_KEY`（若存 GitHub token）

## 9.5 前端部署
1. 配置 API 基地址
2. 接入登录按钮与登录回调处理
3. 接入云端项目与故事列表
4. 启用自动保存与冲突提示
5. 发布上线

---

## 10. GitHub 绑定与文件同步策略

## 10.1 导出到 GitHub（建议路径）
- `stories/<storyId>.narrrail.yaml`

## 10.2 提交策略
- Commit Message 规范：
  - `feat(webeditor): export <storyId> at <timestamp>`
- 支持目标 branch 选择（默认仓库主分支）

## 10.3 失败处理
- GitHub API失败时，不影响云端主存储
- 前端提示“云端已保存，GitHub 同步失败可重试”

---

## 11. 可观测性与运维

1. 错误监控：Sentry（前后端）
2. 接口日志：请求ID + userId + 耗时 + 状态码
3. 关键指标：
   - 登录成功率
   - 自动保存成功率
   - 冲突率（409）
   - GitHub 同步成功率
4. 告警：
   - 5xx 错误异常升高
   - 数据库连接异常
   - OAuth 回调失败率升高

---

## 12. 成本预估（MVP）

- Serverless 后端：免费层可起步，通常 0~5 美元/月
- 数据库：免费层可起步，超量后按存储/请求计费
- 前端托管：免费层可起步
- 总体：小规模验证阶段可控制在低成本

---

## 13. 发布计划建议

## Phase 1（1~2 周）
- GitHub 登录
- 云端故事保存
- 自动保存 + 冲突提示

## Phase 2（1 周）
- GitHub 仓库绑定
- 一键导出到仓库

## Phase 3（2 周）
- 从仓库导入
- 版本比较与恢复
- 只读分享链接

---

## 14. 验收清单（Go-Live Checklist）

- [ ] 未登录用户仅可本地模式
- [ ] 登录后可创建项目与故事
- [ ] 自动保存可跨浏览器会话恢复
- [ ] 不同用户数据隔离验证通过
- [ ] OAuth 回调与 token 交换稳定
- [ ] 关键 API 有鉴权与限流
- [ ] 错误监控已接入
- [ ] 基础备份策略已设置

---

## 15. 风险与回滚

## 15.1 风险
- OAuth 配置错误导致无法登录
- 版本冲突处理不完善导致覆盖误操作
- GitHub API 限流影响同步体验

## 15.2 回滚策略
1. 保留本地缓存兜底
2. 保留 story_versions 快照
3. 同步失败不影响云端主存储
4. 灰度开关可临时关闭 GitHub 同步功能

---

## 16. 结论

该方案能在低运维成本下，快速实现：
- 用户级永久存储
- GitHub 账号绑定登录
- 随时随地编辑与恢复

并可平滑演进到更完整的协作与版本化能力。