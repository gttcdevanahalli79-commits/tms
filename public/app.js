// Train Management System (TMS) - Single Page Web Application Engine
// Comprehensive Karnataka Railway Network Edition

const API_BASE = '/api';

// Global State
let currentUser = JSON.parse(localStorage.getItem('tms_user') || 'null');
let stationsCache = [];
let stationsMeta = { totalKarnatakaStations: 81, districts: [], divisions: [], zones: [] };
let activeBookingTrain = null;
let passengerCount = 1;
let currentCancelPnr = null;

document.addEventListener('DOMContentLoaded', async () => {
    // Set default journey date to tomorrow
    const tomorrow = new Date();
    tomorrow.setDate(tomorrow.getDate() + 1);
    const dateInput = document.getElementById('search-date');
    if (dateInput) {
        dateInput.value = tomorrow.toISOString().split('T')[0];
        dateInput.min = new Date().toISOString().split('T')[0];
    }

    renderAuthUI();
    await loadStations();
    
    // Auto-search default Karnataka corridor: KSR Bengaluru (1) -> Mysuru (25)
    const srcSel = document.getElementById('search-src');
    const dstSel = document.getElementById('search-dst');
    if (srcSel && dstSel) {
        srcSel.value = '1';  // SBC: KSR Bengaluru City
        dstSel.value = '25'; // MYS: Mysuru Junction
        handleSearchTrains();
    }
});

// Toast notification helper
function showToast(message, type = 'success') {
    const toast = document.getElementById('toast');
    const toastMsg = document.getElementById('toast-msg');
    const toastIcon = document.getElementById('toast-icon');

    toast.className = `fixed bottom-6 right-6 z-50 max-w-md p-4 rounded-xl shadow-2xl text-white transform transition-all duration-300 flex items-start space-x-3 ${
        type === 'error' ? 'bg-rose-600' : 'bg-emerald-600'
    }`;
    toastIcon.className = `text-xl mt-0.5 ${type === 'error' ? 'fa-solid fa-triangle-exclamation' : 'fa-solid fa-circle-check'}`;
    toastMsg.textContent = message;
    toast.classList.remove('hidden');

    setTimeout(() => {
        hideToast();
    }, 4500);
}

function hideToast() {
    document.getElementById('toast')?.classList.add('hidden');
}

// ----------------------------------------------------------------------------
// AUTH & SESSION MANAGEMENT
// ----------------------------------------------------------------------------
function renderAuthUI() {
    const container = document.getElementById('auth-container');
    if (!container) return;

    if (currentUser) {
        container.innerHTML = `
            <div class="flex items-center space-x-3">
                <div class="text-right hidden sm:block">
                    <div class="text-xs font-bold text-white flex items-center gap-1.5 justify-end">
                        <span class="w-2 h-2 rounded-full bg-emerald-400"></span>
                        ${escapeHtml(currentUser.fullName)}
                    </div>
                    <div class="text-[10px] text-blue-200 uppercase font-mono">${currentUser.role}</div>
                </div>
                <button onclick="handleLogout()" class="px-3 py-1.5 rounded-lg text-xs font-semibold bg-white/10 hover:bg-rose-600/80 text-white transition border border-white/10">
                    <i class="fa-solid fa-right-from-bracket mr-1"></i> Logout
                </button>
            </div>
        `;
    } else {
        container.innerHTML = `
            <button onclick="openAuthModal()" class="px-4 py-2 rounded-xl text-xs font-bold bg-amber-500 hover:bg-amber-400 text-slate-900 shadow-md transition flex items-center gap-1.5">
                <i class="fa-solid fa-user-lock"></i> Sign In / Register
            </button>
        `;
    }
}

function openAuthModal() {
    document.getElementById('auth-modal')?.classList.remove('hidden');
    showLoginForm();
}

function closeAuthModal() {
    document.getElementById('auth-modal')?.classList.add('hidden');
}

function showLoginForm(e) {
    if (e) e.preventDefault();
    document.getElementById('auth-modal-title').textContent = 'Sign In to TMS';
    document.getElementById('login-form').classList.remove('hidden');
    document.getElementById('register-form').classList.add('hidden');
}

function showRegisterForm(e) {
    if (e) e.preventDefault();
    document.getElementById('auth-modal-title').textContent = 'Create Passenger Account';
    document.getElementById('login-form').classList.add('hidden');
    document.getElementById('register-form').classList.remove('hidden');
}

async function handleLoginSubmit(event) {
    event.preventDefault();
    const username = document.getElementById('login-username').value.trim();
    const password = document.getElementById('login-password').value;
    const expectedRole = document.getElementById('login-role').value;

    try {
        const res = await fetch(`${API_BASE}/auth/login`, {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify({ username, password, expectedRole })
        });
        const data = await res.json();
        if (!res.ok) throw new Error(data.error || 'Login failed');

        currentUser = data.user;
        localStorage.setItem('tms_user', JSON.stringify(currentUser));
        renderAuthUI();
        closeAuthModal();
        showToast(`Welcome back, ${currentUser.fullName}!`);

        if (currentUser.role === 'ADMIN') switchTab('admin');
        else if (currentUser.role === 'OPERATOR') switchTab('operator');
        else switchTab('search');

    } catch (err) {
        showToast(err.message, 'error');
    }
}

async function handleRegisterSubmit(event) {
    event.preventDefault();
    const fullName = document.getElementById('reg-fullname').value.trim();
    const username = document.getElementById('reg-username').value.trim();
    const email = document.getElementById('reg-email').value.trim();
    const phone = document.getElementById('reg-phone').value.trim();
    const password = document.getElementById('reg-password').value;

    try {
        const res = await fetch(`${API_BASE}/auth/register`, {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify({ fullName, username, email, phone, password })
        });
        const data = await res.json();
        if (!res.ok) throw new Error(data.error || 'Registration failed');

        currentUser = data.user;
        localStorage.setItem('tms_user', JSON.stringify(currentUser));
        renderAuthUI();
        closeAuthModal();
        showToast('Registration successful! You are now signed in.');
    } catch (err) {
        showToast(err.message, 'error');
    }
}

function handleLogout() {
    currentUser = null;
    localStorage.removeItem('tms_user');
    renderAuthUI();
    switchTab('search');
    showToast('You have been logged out.');
}

// ----------------------------------------------------------------------------
// TAB NAVIGATION
// ----------------------------------------------------------------------------
function switchTab(tabId) {
    document.querySelectorAll('.tab-content').forEach(el => el.classList.add('hidden'));
    document.querySelectorAll('.nav-btn').forEach(btn => btn.classList.remove('active'));

    const targetSection = document.getElementById(`tab-${tabId}`);
    const targetNav = document.getElementById(`nav-${tabId}`);

    if (targetSection) targetSection.classList.remove('hidden');
    if (targetNav) targetNav.classList.add('active');

    if (tabId === 'stations') loadKarnatakaStationsTab();
    if (tabId === 'bookings') loadMyBookings();
    if (tabId === 'operator') loadOperatorSchedules();
    if (tabId === 'admin') loadAdminDashboard();
}

// ----------------------------------------------------------------------------
// KARNATAKA STATIONS DIRECTORY & MANAGEMENT
// ----------------------------------------------------------------------------
async function loadStations() {
    try {
        // Fetch metadata
        const metaRes = await fetch(`${API_BASE}/stations/meta`);
        stationsMeta = await metaRes.json();

        // Update badge counts in directory header if present
        const totalCountEl = document.getElementById('kt-total-count');
        const distCountEl = document.getElementById('kt-district-count');
        const divCountEl = document.getElementById('kt-division-count');
        if (totalCountEl) totalCountEl.textContent = stationsMeta.totalKarnatakaStations;
        if (distCountEl) distCountEl.textContent = stationsMeta.districts.length;
        if (divCountEl) divCountEl.textContent = stationsMeta.divisions.length;

        // Populate directory filter dropdowns
        const distFilter = document.getElementById('filter-station-district');
        const divFilter = document.getElementById('filter-station-division');
        if (distFilter) {
            distFilter.innerHTML = `<option value="">All Districts (${stationsMeta.districts.length})</option>` +
                stationsMeta.districts.map(d => `<option value="${d}">${d}</option>`).join('');
        }
        if (divFilter) {
            divFilter.innerHTML = `<option value="">All Railway Divisions (${stationsMeta.divisions.length})</option>` +
                stationsMeta.divisions.map(d => `<option value="${d}">${d}</option>`).join('');
        }

        // Fetch stations list
        const res = await fetch(`${API_BASE}/stations?sort=asc`);
        stationsCache = await res.json();

        const srcSel = document.getElementById('search-src');
        const dstSel = document.getElementById('search-dst');
        if (!srcSel || !dstSel) return;

        srcSel.innerHTML = '<option value="">Select Origin Station</option>';
        dstSel.innerHTML = '<option value="">Select Destination Station</option>';

        stationsCache.forEach(s => {
            const label = `${s.name} (${s.code}) - ${s.district} Dist`;
            const opt1 = new Option(label, s.station_id);
            const opt2 = new Option(label, s.station_id);
            srcSel.add(opt1);
            dstSel.add(opt2);
        });

    } catch (err) {
        console.error('Failed to load stations:', err);
    }
}

async function loadKarnatakaStationsTab() {
    await filterKarnatakaStations();
}

async function filterKarnatakaStations() {
    const search = document.getElementById('filter-station-search')?.value.trim() || '';
    const district = document.getElementById('filter-station-district')?.value || '';
    const division = document.getElementById('filter-station-division')?.value || '';
    const sort = document.getElementById('filter-station-sort')?.value || 'asc';

    const tbody = document.getElementById('karnataka-stations-tbody');
    if (!tbody) return;

    tbody.innerHTML = `<tr><td colspan="7" class="p-8 text-center text-slate-500"><i class="fa-solid fa-spinner fa-spin text-2xl text-amber-600 mb-2"></i><div>Searching Karnataka railway stations...</div></td></tr>`;

    try {
        const queryParams = new URLSearchParams({ sort });
        if (search) queryParams.set('search', search);
        if (district) queryParams.set('district', district);
        if (division) queryParams.set('division', division);

        const res = await fetch(`${API_BASE}/stations?${queryParams.toString()}`);
        const stations = await res.json();

        const countBadge = document.getElementById('filtered-count');
        if (countBadge) countBadge.textContent = stations.length;

        if (stations.length === 0) {
            tbody.innerHTML = `<tr><td colspan="7" class="p-8 text-center text-slate-400">No Karnataka stations match your search filters.</td></tr>`;
            return;
        }

        tbody.innerHTML = stations.map(s => `
            <tr class="hover:bg-slate-50 transition">
                <td class="p-3 font-mono font-bold text-amber-800 bg-amber-50/50">
                    <span class="px-2 py-0.5 rounded border border-amber-200 bg-white">${s.code}</span>
                </td>
                <td class="p-3">
                    <div class="font-bold text-slate-800">${escapeHtml(s.name)}</div>
                    <div class="text-[11px] text-slate-400">${s.city}, Karnataka</div>
                </td>
                <td class="p-3">
                    <span class="font-semibold text-slate-700 bg-slate-100 px-2 py-0.5 rounded">${s.district}</span>
                </td>
                <td class="p-3">
                    <div class="text-xs font-semibold text-blue-900">${s.railway_division} Division</div>
                    <div class="text-[10px] text-slate-500">${s.railway_zone}</div>
                </td>
                <td class="p-3 text-center font-bold text-slate-700">${s.platforms_count}</td>
                <td class="p-3 text-slate-400 font-mono text-[11px]">
                    ${s.latitude && s.longitude ? `${s.latitude.toFixed(2)}° N, ${s.longitude.toFixed(2)}° E` : '—'}
                </td>
                <td class="p-3 text-right space-x-1.5 whitespace-nowrap">
                    <button onclick="setRouteOrigin(${s.station_id})" class="px-2.5 py-1 bg-blue-50 text-blue-700 hover:bg-blue-100 border border-blue-200 rounded-lg text-xs font-semibold transition" title="Set as origin">
                        <i class="fa-solid fa-plane-departure mr-1"></i> From
                    </button>
                    <button onclick="setRouteDest(${s.station_id})" class="px-2.5 py-1 bg-amber-50 text-amber-700 hover:bg-amber-100 border border-amber-200 rounded-lg text-xs font-semibold transition" title="Set as destination">
                        <i class="fa-solid fa-plane-arrival mr-1"></i> To
                    </button>
                </td>
            </tr>
        `).join('');

    } catch (err) {
        tbody.innerHTML = `<tr><td colspan="7" class="p-4 text-rose-600">${err.message}</td></tr>`;
    }
}

function resetStationFilters() {
    const s = document.getElementById('filter-station-search');
    const d = document.getElementById('filter-station-district');
    const v = document.getElementById('filter-station-division');
    const o = document.getElementById('filter-station-sort');
    if (s) s.value = '';
    if (d) d.value = '';
    if (v) v.value = '';
    if (o) o.value = 'asc';
    filterKarnatakaStations();
}

function setRouteOrigin(stationId) {
    switchTab('search');
    const srcSel = document.getElementById('search-src');
    if (srcSel) srcSel.value = String(stationId);
    showToast('Origin station selected. Now choose your destination.');
}

function setRouteDest(stationId) {
    switchTab('search');
    const dstSel = document.getElementById('search-dst');
    if (dstSel) dstSel.value = String(stationId);
    showToast('Destination station selected.');
    handleSearchTrains();
}

// ----------------------------------------------------------------------------
// TRAIN SEARCH & DISPLAY
// ----------------------------------------------------------------------------
async function handleSearchTrains(e) {
    if (e) e.preventDefault();

    const src = document.getElementById('search-src').value;
    const dst = document.getElementById('search-dst').value;
    const date = document.getElementById('search-date').value;
    const coachClass = document.getElementById('search-class').value;

    if (!src || !dst) {
        showToast('Please select both Origin and Destination stations.', 'error');
        return;
    }
    if (src === dst) {
        showToast('Origin and Destination stations cannot be identical.', 'error');
        return;
    }

    const container = document.getElementById('trains-list');
    container.innerHTML = `
        <div class="bg-white p-8 rounded-2xl border text-center text-slate-500">
            <i class="fa-solid fa-spinner fa-spin text-2xl text-blue-600 mb-2"></i>
            <div>Searching trains across Karnataka corridors and calculating dynamic fares...</div>
        </div>
    `;

    try {
        const res = await fetch(`${API_BASE}/trains/search?source=${src}&dest=${dst}&date=${date}&coachClass=${coachClass}`);
        const trains = await res.json();

        const countBadge = document.getElementById('results-count');
        countBadge.textContent = `${trains.length} Train${trains.length === 1 ? '' : 's'} Found`;

        if (!trains || trains.length === 0) {
            container.innerHTML = `
                <div class="bg-white p-12 rounded-2xl border text-center text-slate-500 space-y-3">
                    <i class="fa-solid fa-train-slash text-4xl text-slate-300"></i>
                    <h3 class="text-base font-bold text-slate-700">No Direct Trains Found Between Selected Karnataka Stations</h3>
                    <p class="text-xs text-slate-400">Popular routes: KSR Bengaluru (SBC) to Mysuru (MYS), Hubballi (UBL), Belagavi (BGM), or Shivamogga (SMET).</p>
                </div>
            `;
            return;
        }

        container.innerHTML = trains.map(t => renderTrainCard(t, date)).join('');

    } catch (err) {
        container.innerHTML = `<div class="p-6 bg-rose-50 text-rose-700 rounded-xl text-sm">${err.message}</div>`;
    }
}

function renderTrainCard(train, journeyDate) {
    const badgeClass = `badge-${train.type.toLowerCase().replace('_', '-')}`;
    const delayBadge = train.delayMinutes > 0
        ? `<span class="text-xs bg-rose-100 text-rose-700 px-2 py-0.5 rounded-full font-semibold">Delayed by ${train.delayMinutes} mins</span>`
        : `<span class="text-xs bg-emerald-100 text-emerald-700 px-2 py-0.5 rounded-full font-semibold"><i class="fa-solid fa-circle-check mr-1"></i>On Time</span>`;

    return `
        <div class="train-card bg-white rounded-2xl border border-slate-200 p-6 shadow-sm flex flex-col md:flex-row md:items-center justify-between gap-6">
            <div class="space-y-3 flex-1">
                <div class="flex items-center gap-3 flex-wrap">
                    <span class="font-mono text-base font-bold text-blue-900 bg-blue-50 px-2.5 py-1 rounded-lg border border-blue-200">
                        #${train.trainNumber}
                    </span>
                    <h3 class="text-lg font-bold text-slate-800">${escapeHtml(train.trainName)}</h3>
                    <span class="text-xs px-2.5 py-1 rounded-full font-bold uppercase tracking-wider ${badgeClass}">
                        ${train.type}
                    </span>
                    ${delayBadge}
                    ${train.isOvernight ? '<span class="text-xs bg-indigo-50 text-indigo-700 px-2 py-0.5 rounded-full"><i class="fa-solid fa-moon mr-1"></i>Overnight</span>' : ''}
                </div>

                <!-- Schedule & Station Corridor -->
                <div class="flex items-center gap-4 text-sm font-medium text-slate-700">
                    <div>
                        <div class="text-xl font-extrabold text-slate-900">${train.departureTime}</div>
                        <div class="text-xs text-slate-500 font-semibold">${train.source.code} - ${train.source.name}</div>
                    </div>
                    <div class="flex-1 flex flex-col items-center px-4 max-w-xs">
                        <div class="text-[11px] text-slate-400 font-semibold mb-1">${train.distanceKm} km (Karnataka Corridor)</div>
                        <div class="w-full flex items-center">
                            <div class="w-2 h-2 rounded-full bg-blue-600"></div>
                            <div class="flex-1 border-t-2 border-dashed border-slate-300 mx-1"></div>
                            <i class="fa-solid fa-train text-xs text-blue-600"></i>
                            <div class="flex-1 border-t-2 border-dashed border-slate-300 mx-1"></div>
                            <div class="w-2 h-2 rounded-full bg-slate-800"></div>
                        </div>
                    </div>
                    <div>
                        <div class="text-xl font-extrabold text-slate-900">${train.arrivalTime}</div>
                        <div class="text-xs text-slate-500 font-semibold">${train.destination.code} - ${train.destination.name}</div>
                    </div>
                </div>

                <!-- Available classes tags -->
                <div class="flex items-center gap-2 pt-1 text-xs">
                    <span class="text-slate-400 font-semibold">Classes Available:</span>
                    ${train.availableClasses.map(c => `
                        <span class="px-2 py-0.5 rounded-md font-semibold ${c === train.coachClass ? 'bg-blue-600 text-white' : 'bg-slate-100 text-slate-600'}">
                            ${c}
                        </span>
                    `).join('')}
                </div>
            </div>

            <!-- Booking & Pricing Column -->
            <div class="flex flex-row md:flex-col items-center md:items-end justify-between border-t md:border-t-0 md:border-l border-slate-100 pt-4 md:pt-0 md:pl-6 min-w-[200px]">
                <div class="text-left md:text-right">
                    <div class="text-xs text-slate-400 uppercase font-semibold">Class: ${train.coachClass} Fare</div>
                    <div class="text-2xl font-extrabold text-slate-900">Rs. ${train.fareRupees.toFixed(2)}</div>
                    <div class="text-[11px] text-emerald-600 font-semibold flex items-center md:justify-end gap-1 mt-0.5">
                        <i class="fa-solid fa-chair"></i>
                        <span>${train.availableSeats} Seats Available</span>
                    </div>
                </div>

                <button onclick="openBookingModal(${JSON.stringify(train).replace(/"/g, '&quot;')}, '${journeyDate}')"
                        class="mt-3 bg-blue-600 hover:bg-blue-700 text-white font-bold px-6 py-2.5 rounded-xl shadow-md transition flex items-center gap-2">
                    <span>Book Now</span>
                    <i class="fa-solid fa-arrow-right text-xs"></i>
                </button>
            </div>
        </div>
    `;
}

// ----------------------------------------------------------------------------
// BOOKING MODAL & MULTI-PASSENGER FORM
// ----------------------------------------------------------------------------
function openBookingModal(train, journeyDate) {
    if (!currentUser) {
        showToast('Please sign in or register to book tickets.', 'error');
        openAuthModal();
        return;
    }

    activeBookingTrain = { ...train, journeyDate };
    passengerCount = 0;

    document.getElementById('modal-train-title').textContent = `#${train.trainNumber} - ${train.trainName}`;
    document.getElementById('modal-train-subtitle').textContent = `${train.source.code} to ${train.destination.code} | Date: ${journeyDate} | Class: ${train.coachClass}`;

    const container = document.getElementById('passengers-container');
    container.innerHTML = '';

    // Add first passenger row by default
    addPassengerRow(currentUser.fullName, 28, 'M', 'Lower');
    updateFareSummary();

    document.getElementById('booking-modal')?.classList.remove('hidden');
}

function closeBookingModal() {
    document.getElementById('booking-modal')?.classList.add('hidden');
    activeBookingTrain = null;
}

function addPassengerRow(defaultName = '', defaultAge = '', defaultGender = 'M', defaultBerth = 'Lower') {
    if (passengerCount >= 6) {
        showToast('Maximum 6 passengers allowed per reservation.', 'error');
        return;
    }

    passengerCount++;
    const container = document.getElementById('passengers-container');
    const rowId = `passenger-row-${passengerCount}`;

    const row = document.createElement('div');
    row.id = rowId;
    row.className = 'p-3 bg-slate-50 border border-slate-200 rounded-xl space-y-2 relative';
    row.innerHTML = `
        <div class="flex items-center justify-between text-xs font-bold text-slate-700">
            <span>Passenger #${passengerCount}</span>
            ${passengerCount > 1 ? `<button type="button" onclick="removePassengerRow('${rowId}')" class="text-rose-500 hover:text-rose-700 text-xs"><i class="fa-solid fa-trash"></i></button>` : ''}
        </div>
        <div class="grid grid-cols-1 sm:grid-cols-4 gap-2">
            <div class="sm:col-span-2">
                <input type="text" placeholder="Full Name" value="${defaultName}" class="p-name w-full px-3 py-1.5 rounded-lg border border-slate-300 text-xs bg-white font-medium" required />
            </div>
            <div>
                <input type="number" placeholder="Age" min="1" max="120" value="${defaultAge}" class="p-age w-full px-3 py-1.5 rounded-lg border border-slate-300 text-xs bg-white font-medium" required />
            </div>
            <div>
                <select class="p-gender w-full px-2 py-1.5 rounded-lg border border-slate-300 text-xs bg-white font-medium">
                    <option value="M" ${defaultGender === 'M' ? 'selected' : ''}>Male</option>
                    <option value="F" ${defaultGender === 'F' ? 'selected' : ''}>Female</option>
                    <option value="O" ${defaultGender === 'O' ? 'selected' : ''}>Other</option>
                </select>
            </div>
        </div>
        <div class="flex items-center gap-2 text-xs">
            <span class="text-slate-500">Berth Preference:</span>
            <select class="p-berth px-2 py-1 rounded-lg border border-slate-300 text-xs bg-white font-medium">
                <option value="Lower" ${defaultBerth === 'Lower' ? 'selected' : ''}>Lower</option>
                <option value="Middle" ${defaultBerth === 'Middle' ? 'selected' : ''}>Middle</option>
                <option value="Upper" ${defaultBerth === 'Upper' ? 'selected' : ''}>Upper</option>
                <option value="Side Lower" ${defaultBerth === 'Side Lower' ? 'selected' : ''}>Side Lower</option>
                <option value="Side Upper" ${defaultBerth === 'Side Upper' ? 'selected' : ''}>Side Upper</option>
                <option value="Window">Window Seat (CC)</option>
                <option value="Aisle">Aisle Seat</option>
            </select>
        </div>
    `;

    container.appendChild(row);
    updateFareSummary();
}

function removePassengerRow(rowId) {
    const el = document.getElementById(rowId);
    if (el) el.remove();
    passengerCount = document.querySelectorAll('#passengers-container > div').length;
    updateFareSummary();
}

function updateFareSummary() {
    if (!activeBookingTrain) return;

    const count = document.querySelectorAll('#passengers-container > div').length;
    const single = activeBookingTrain.fareRupees;
    const total = single * count;

    document.getElementById('summary-single-fare').textContent = `Rs. ${single.toFixed(2)}`;
    document.getElementById('summary-passenger-count').textContent = count;
    document.getElementById('summary-total-fare').textContent = `Rs. ${total.toFixed(2)}`;
}

async function confirmBooking() {
    if (!activeBookingTrain || !currentUser) return;

    const rows = document.querySelectorAll('#passengers-container > div');
    const passengers = [];

    for (const r of rows) {
        const name = r.querySelector('.p-name').value.trim();
        const age = parseInt(r.querySelector('.p-age').value, 10);
        const gender = r.querySelector('.p-gender').value;
        const berthPreference = r.querySelector('.p-berth').value;

        if (!name || isNaN(age)) {
            showToast('Please fill out name and age for all passengers.', 'error');
            return;
        }
        passengers.push({ name, age, gender, berthPreference });
    }

    const paymentMethod = document.querySelector('input[name="payment-method"]:checked')?.value || 'UPI';
    const btn = document.getElementById('confirm-booking-btn');
    btn.disabled = true;
    btn.innerHTML = '<i class="fa-solid fa-spinner fa-spin mr-1"></i> Processing Booking...';

    try {
        const res = await fetch(`${API_BASE}/bookings`, {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify({
                userId: currentUser.userId,
                trainId: activeBookingTrain.trainId,
                journeyDate: activeBookingTrain.journeyDate,
                sourceStationId: activeBookingTrain.source.id,
                destStationId: activeBookingTrain.destination.id,
                coachClass: activeBookingTrain.coachClass,
                passengers,
                paymentMethod
            })
        });

        const data = await res.json();
        if (!res.ok) throw new Error(data.error || 'Booking reservation failed');

        closeBookingModal();
        showToast(`Ticket Booked! PNR: ${data.booking.pnr}`);
        showTicketSlip(data.booking);

    } catch (err) {
        showToast(err.message, 'error');
    } finally {
        btn.disabled = false;
        btn.innerHTML = 'Confirm & Pay';
    }
}

// ----------------------------------------------------------------------------
// ELECTRONIC RESERVATION SLIP (ERS) DISPLAY
// ----------------------------------------------------------------------------
function showTicketSlip(booking) {
    const modal = document.getElementById('ticket-modal');
    const content = document.getElementById('ticket-content');

    content.innerHTML = `
        <div class="border-b pb-3 space-y-1">
            <div class="flex justify-between items-center">
                <span class="text-slate-500 font-semibold uppercase">10-Digit PNR Number:</span>
                <span class="font-mono text-base font-extrabold text-blue-900 bg-blue-100 px-3 py-0.5 rounded-lg border border-blue-300">
                    ${booking.pnr}
                </span>
            </div>
            <div class="flex justify-between">
                <span class="text-slate-500">Date of Journey:</span>
                <span class="font-bold text-slate-800">${booking.journeyDate}</span>
            </div>
            <div class="flex justify-between">
                <span class="text-slate-500">Class:</span>
                <span class="font-bold text-slate-800">${booking.coachClass}</span>
            </div>
            <div class="flex justify-between">
                <span class="text-slate-500">Status:</span>
                <span class="font-bold text-emerald-700 bg-emerald-100 px-2 py-0.5 rounded-full">${booking.status}</span>
            </div>
            <div class="flex justify-between">
                <span class="text-slate-500">Simulated Txn ID:</span>
                <span class="font-mono text-slate-700">${booking.transactionId} (${booking.paymentMethod})</span>
            </div>
        </div>

        <div class="space-y-2">
            <div class="font-bold text-slate-700">Passenger Seat Allocations:</div>
            ${booking.passengers.map((p, idx) => `
                <div class="flex justify-between items-center bg-white p-2.5 rounded-lg border border-slate-200">
                    <div>
                        <span class="font-bold text-slate-800">${idx + 1}. ${escapeHtml(p.name)}</span>
                        <span class="text-[10px] text-slate-400 ml-1">(${p.age}y, ${p.gender})</span>
                    </div>
                    <div class="font-mono font-bold text-blue-800 bg-blue-50 px-2 py-1 rounded border border-blue-200 text-xs">
                        Coach ${p.coachCode || p.allocated_coach_code} / Seat ${p.seatNumber || p.allocated_seat_number} (${p.berthType || p.allocated_berth_type})
                    </div>
                </div>
            `).join('')}
        </div>

        <div class="border-t pt-2 flex justify-between font-bold text-sm text-slate-900">
            <span>Total Fare Paid:</span>
            <span class="text-blue-800">Rs. ${booking.totalFareRupees.toFixed(2)}</span>
        </div>
    `;

    modal.classList.remove('hidden');
}

function closeTicketModal() {
    document.getElementById('ticket-modal')?.classList.add('hidden');
}

// ----------------------------------------------------------------------------
// PNR STATUS & CANCELLATION
// ----------------------------------------------------------------------------
async function handleCheckPNR(e) {
    if (e) e.preventDefault();
    const pnr = document.getElementById('pnr-input').value.trim();

    if (!pnr || pnr.length !== 10) {
        showToast('Please enter a valid 10-digit PNR number.', 'error');
        return;
    }

    const card = document.getElementById('pnr-result-card');
    card.classList.remove('hidden');
    card.innerHTML = `<div class="p-8 text-center text-slate-500"><i class="fa-solid fa-spinner fa-spin text-2xl text-blue-600 mb-2"></i><div>Fetching PNR details...</div></div>`;

    try {
        const res = await fetch(`${API_BASE}/bookings/${pnr}`);
        const data = await res.json();
        if (!res.ok) throw new Error(data.error || 'PNR lookup failed');

        currentCancelPnr = pnr;
        const isCancelled = data.status === 'CANCELLED';

        card.innerHTML = `
            <div class="bg-white rounded-2xl border border-slate-200 shadow-md p-6 space-y-6">
                <div class="flex items-start justify-between flex-wrap gap-4 border-b border-slate-100 pb-4">
                    <div>
                        <div class="text-xs uppercase font-semibold text-slate-400">PNR Status Record</div>
                        <div class="text-2xl font-mono font-extrabold text-blue-900 mt-0.5">${data.pnr}</div>
                        <div class="text-sm font-semibold text-slate-700 mt-1">#${data.train_number} - ${data.train_name}</div>
                    </div>
                    <div class="text-right">
                        <span class="px-3 py-1 rounded-full text-xs font-bold uppercase tracking-wider ${
                            isCancelled ? 'bg-rose-100 text-rose-700 border border-rose-200' : 'bg-emerald-100 text-emerald-700 border border-emerald-200'
                        }">
                            ${data.status}
                        </span>
                        <div class="text-xs text-slate-500 mt-1.5 font-semibold">Total Fare: Rs. ${data.totalFareRupees.toFixed(2)}</div>
                    </div>
                </div>

                <div class="grid grid-cols-2 sm:grid-cols-4 gap-4 text-xs">
                    <div class="bg-slate-50 p-3 rounded-xl border">
                        <div class="text-slate-400">Journey Date</div>
                        <div class="font-bold text-slate-800 text-sm mt-0.5">${data.journey_date}</div>
                    </div>
                    <div class="bg-slate-50 p-3 rounded-xl border">
                        <div class="text-slate-400">Origin Station</div>
                        <div class="font-bold text-slate-800 text-sm mt-0.5">${data.src_code} - ${data.src_name}</div>
                    </div>
                    <div class="bg-slate-50 p-3 rounded-xl border">
                        <div class="text-slate-400">Destination</div>
                        <div class="font-bold text-slate-800 text-sm mt-0.5">${data.dst_code} - ${data.dst_name}</div>
                    </div>
                    <div class="bg-slate-50 p-3 rounded-xl border">
                        <div class="text-slate-400">Class</div>
                        <div class="font-bold text-slate-800 text-sm mt-0.5">${data.coach_class}</div>
                    </div>
                </div>

                <!-- Passengers and seats -->
                <div class="space-y-3">
                    <h4 class="font-bold text-sm text-slate-800">Passenger Berths & Allocations</h4>
                    <div class="space-y-2">
                        ${data.passengers.map((p, i) => `
                            <div class="flex justify-between items-center p-3 rounded-xl bg-slate-50 border border-slate-200 text-xs">
                                <div>
                                    <span class="font-bold text-slate-900">${i + 1}. ${escapeHtml(p.name)}</span>
                                    <span class="text-slate-400 ml-1">(${p.age}y, ${p.gender})</span>
                                </div>
                                <div class="font-mono font-bold text-blue-900 bg-blue-100/60 px-2.5 py-1 rounded-lg border border-blue-200">
                                    Coach ${p.allocated_coach_code} / Berth #${p.allocated_seat_number} (${p.allocated_berth_type})
                                </div>
                            </div>
                        `).join('')}
                    </div>
                </div>

                ${data.refund ? `
                    <div class="bg-rose-50 border border-rose-200 rounded-xl p-4 text-xs space-y-1">
                        <div class="font-bold text-rose-800 flex items-center gap-1.5">
                            <i class="fa-solid fa-receipt"></i> Refund Processed
                        </div>
                        <div class="text-rose-700">Amount Refunded: <b>Rs. ${data.refund.refundAmountRupees.toFixed(2)}</b> (${data.refund.deduction_percentage}% deduction applied).</div>
                        <div class="text-[11px] text-rose-600">${data.refund.reason}</div>
                    </div>
                ` : ''}

                <!-- Actions -->
                <div class="flex justify-end gap-3 pt-2">
                    ${!isCancelled ? `
                        <button onclick="openCancelModal('${data.pnr}', ${data.totalFareRupees})"
                                class="bg-rose-600 hover:bg-rose-700 text-white font-bold px-5 py-2.5 rounded-xl text-xs shadow-md transition flex items-center gap-1.5">
                            <i class="fa-solid fa-ban"></i> Cancel Ticket & Request Refund
                        </button>
                    ` : ''}
                    <button onclick="window.print()" class="bg-slate-100 hover:bg-slate-200 text-slate-700 font-semibold px-4 py-2.5 rounded-xl text-xs transition">
                        <i class="fa-solid fa-print mr-1"></i> Print Slip
                    </button>
                </div>
            </div>
        `;

    } catch (err) {
        card.innerHTML = `<div class="p-6 bg-rose-50 border border-rose-200 text-rose-700 rounded-2xl text-sm font-semibold">${err.message}</div>`;
    }
}

function openCancelModal(pnr, fare) {
    currentCancelPnr = pnr;
    document.getElementById('cancel-modal-desc').textContent = `Ticket PNR: ${pnr} (Total Fare: Rs. ${fare.toFixed(2)})`;
    document.getElementById('cancel-modal')?.classList.remove('hidden');
}

function closeCancelModal() {
    document.getElementById('cancel-modal')?.classList.add('hidden');
    currentCancelPnr = null;
}

async function executeCancellation() {
    if (!currentCancelPnr) return;

    const hours = parseFloat(document.getElementById('cancel-hours').value) || 36;

    try {
        const res = await fetch(`${API_BASE}/bookings/${currentCancelPnr}/cancel`, {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify({
                hoursBeforeDeparture: hours,
                reason: 'Passenger cancellation request',
                userId: currentUser ? currentUser.userId : null
            })
        });

        const data = await res.json();
        if (!res.ok) throw new Error(data.error || 'Cancellation failed');

        closeCancelModal();
        showToast(`Ticket cancelled! Refund of Rs. ${data.refundAmountRupees.toFixed(2)} generated.`);
        document.getElementById('pnr-input').value = currentCancelPnr;
        handleCheckPNR();

    } catch (err) {
        showToast(err.message, 'error');
    }
}

// ----------------------------------------------------------------------------
// MY BOOKINGS
// ----------------------------------------------------------------------------
async function loadMyBookings() {
    if (!currentUser) {
        document.getElementById('my-bookings-list').innerHTML = `
            <div class="bg-white p-12 rounded-2xl border text-center text-slate-500 space-y-3">
                <i class="fa-solid fa-user-lock text-4xl text-slate-300"></i>
                <h3 class="font-bold text-slate-700">Please Sign In</h3>
                <p class="text-xs text-slate-400">Sign in to view your reservation history and manage tickets.</p>
                <button onclick="openAuthModal()" class="px-4 py-2 bg-blue-600 text-white rounded-xl text-xs font-bold shadow-md">Sign In</button>
            </div>
        `;
        return;
    }

    const list = document.getElementById('my-bookings-list');
    list.innerHTML = `<div class="p-8 text-center text-slate-500"><i class="fa-solid fa-spinner fa-spin text-2xl text-blue-600"></i></div>`;

    try {
        const res = await fetch(`${API_BASE}/bookings/user/${currentUser.userId}`);
        const bookings = await res.json();

        if (!bookings || bookings.length === 0) {
            list.innerHTML = `
                <div class="bg-white p-12 rounded-2xl border text-center text-slate-500 space-y-3">
                    <i class="fa-solid fa-ticket text-4xl text-slate-300"></i>
                    <h3 class="font-bold text-slate-700">No Bookings Yet</h3>
                    <p class="text-xs text-slate-400">Search for trains to book your first Karnataka journey!</p>
                    <button onclick="switchTab('search')" class="px-4 py-2 bg-blue-600 text-white rounded-xl text-xs font-bold">Search Trains</button>
                </div>
            `;
            return;
        }

        list.innerHTML = bookings.map(b => `
            <div class="bg-white p-5 rounded-2xl border border-slate-200 shadow-sm flex flex-col sm:flex-row sm:items-center justify-between gap-4">
                <div class="space-y-1">
                    <div class="flex items-center gap-2">
                        <span class="font-mono font-bold text-blue-900 bg-blue-50 px-2.5 py-0.5 rounded border border-blue-200 text-sm">
                            PNR: ${b.pnr}
                        </span>
                        <span class="text-xs font-bold px-2 py-0.5 rounded-full ${b.status === 'CONFIRMED' ? 'bg-emerald-100 text-emerald-800' : 'bg-rose-100 text-rose-800'}">
                            ${b.status}
                        </span>
                    </div>
                    <div class="text-sm font-bold text-slate-800">
                        #${b.train_number} - ${b.train_name} (${b.src_code} -> ${b.dst_code})
                    </div>
                    <div class="text-xs text-slate-500">
                        Date: <b>${b.journey_date}</b> | Class: <b>${b.coach_class}</b> | ${b.passengers.length} Passenger(s)
                    </div>
                </div>

                <div class="flex items-center gap-4 justify-between sm:justify-end">
                    <div class="text-right">
                        <div class="text-xs text-slate-400">Fare Paid</div>
                        <div class="text-base font-extrabold text-slate-900">Rs. ${b.totalFareRupees.toFixed(2)}</div>
                    </div>
                    <button onclick="inspectBooking('${b.pnr}')" class="px-3.5 py-2 bg-slate-100 hover:bg-slate-200 text-slate-700 font-semibold rounded-xl text-xs transition">
                        View Details
                    </button>
                </div>
            </div>
        `).join('');

    } catch (err) {
        list.innerHTML = `<div class="p-6 bg-rose-50 text-rose-700 rounded-xl text-sm">${err.message}</div>`;
    }
}

function inspectBooking(pnr) {
    switchTab('pnr');
    document.getElementById('pnr-input').value = pnr;
    handleCheckPNR();
}

// ----------------------------------------------------------------------------
// OPERATOR CONSOLE
// ----------------------------------------------------------------------------
async function loadOperatorSchedules() {
    const tbody = document.getElementById('operator-schedules-body');
    tbody.innerHTML = `<tr><td colspan="6" class="p-8 text-center text-slate-500"><i class="fa-solid fa-spinner fa-spin text-2xl text-emerald-600"></i></td></tr>`;

    try {
        const res = await fetch(`${API_BASE}/operator/schedules`);
        const schedules = await res.json();

        tbody.innerHTML = schedules.map(s => `
            <tr class="hover:bg-slate-50 transition">
                <td class="p-3">
                    <div class="font-bold text-slate-800">#${s.train_number}</div>
                    <div class="text-xs text-slate-500">${s.train_name}</div>
                </td>
                <td class="p-3 font-semibold text-slate-700">${s.src_code} &rarr; ${s.dst_code}</td>
                <td class="p-3">
                    <span class="font-bold text-slate-800">${s.departure_time}</span>
                    <span class="text-slate-400 text-xs">/ ${s.arrival_time}</span>
                </td>
                <td class="p-3 text-xs text-slate-600">${s.operating_days}</td>
                <td class="p-3">
                    ${s.delay_minutes > 0
                        ? `<span class="bg-rose-100 text-rose-800 font-bold px-2 py-0.5 rounded text-xs">Delayed ${s.delay_minutes}m</span>`
                        : `<span class="bg-emerald-100 text-emerald-800 font-bold px-2 py-0.5 rounded text-xs">On Time</span>`}
                </td>
                <td class="p-3 text-right space-x-2">
                    <button onclick="promptUpdateDelay(${s.schedule_id}, ${s.delay_minutes})" class="px-2.5 py-1 bg-amber-50 text-amber-700 hover:bg-amber-100 border border-amber-200 rounded-lg text-xs font-semibold">
                        <i class="fa-solid fa-clock mr-1"></i> Update Delay
                    </button>
                    <button onclick="viewPassengerManifest(${s.train_id}, '${escapeHtml(s.train_name)}')" class="px-2.5 py-1 bg-emerald-50 text-emerald-700 hover:bg-emerald-100 border border-emerald-200 rounded-lg text-xs font-semibold">
                        <i class="fa-solid fa-users mr-1"></i> Manifest
                    </button>
                </td>
            </tr>
        `).join('');

    } catch (err) {
        tbody.innerHTML = `<tr><td colspan="6" class="p-4 text-rose-600">${err.message}</td></tr>`;
    }
}

async function promptUpdateDelay(scheduleId, currentDelay) {
    const minutes = prompt(`Enter delay in minutes for schedule #${scheduleId}:`, currentDelay);
    if (minutes === null) return;

    try {
        const res = await fetch(`${API_BASE}/operator/delay`, {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify({
                scheduleId,
                delayMinutes: parseInt(minutes, 10) || 0,
                operatorUserId: currentUser ? currentUser.userId : 2
            })
        });
        if (!res.ok) throw new Error('Failed to update delay');
        showToast('Train running delay updated successfully.');
        loadOperatorSchedules();
    } catch (err) {
        showToast(err.message, 'error');
    }
}

async function viewPassengerManifest(trainId, trainName) {
    const container = document.getElementById('operator-manifest-container');
    container.classList.remove('hidden');
    container.innerHTML = `<div class="text-center p-6 text-slate-500"><i class="fa-solid fa-spinner fa-spin"></i> Loading chart manifest...</div>`;

    try {
        const res = await fetch(`${API_BASE}/operator/manifest/${trainId}`);
        const passengers = await res.json();

        container.innerHTML = `
            <div class="flex items-center justify-between border-b pb-3">
                <div>
                    <h4 class="font-bold text-slate-800 text-base">Passenger Chart & Manifest: ${trainName}</h4>
                    <div class="text-xs text-slate-500">Confirmed seat reservations for today's Karnataka run</div>
                </div>
                <button onclick="document.getElementById('operator-manifest-container').classList.add('hidden')" class="text-slate-400 hover:text-slate-600">&times;</button>
            </div>

            ${passengers.length === 0 ? `
                <div class="text-center p-8 text-slate-400 text-xs">No confirmed passenger bookings recorded for today's run.</div>
            ` : `
                <div class="overflow-x-auto">
                    <table class="w-full text-left text-xs">
                        <thead class="bg-slate-50 text-slate-600 uppercase font-semibold">
                            <tr>
                                <th class="p-2.5">Coach & Seat</th>
                                <th class="p-2.5">Passenger Name</th>
                                <th class="p-2.5">Age / Gender</th>
                                <th class="p-2.5">Berth Type</th>
                                <th class="p-2.5">PNR</th>
                                <th class="p-2.5">Corridor</th>
                            </tr>
                        </thead>
                        <tbody class="divide-y divide-slate-100">
                            ${passengers.map(p => `
                                <tr>
                                    <td class="p-2.5 font-bold font-mono text-blue-900">${p.allocated_coach_code}-${p.allocated_seat_number}</td>
                                    <td class="p-2.5 font-semibold text-slate-800">${escapeHtml(p.name)}</td>
                                    <td class="p-2.5 text-slate-500">${p.age}y / ${p.gender}</td>
                                    <td class="p-2.5">${p.allocated_berth_type}</td>
                                    <td class="p-2.5 font-mono text-slate-600">${p.pnr}</td>
                                    <td class="p-2.5 text-slate-500">${p.src_code} &rarr; ${p.dst_code}</td>
                                </tr>
                            `).join('')}
                        </tbody>
                    </table>
                </div>
            `}
        `;
    } catch (err) {
        container.innerHTML = `<div class="p-4 text-rose-600">${err.message}</div>`;
    }
}

// ----------------------------------------------------------------------------
// ADMIN DASHBOARD
// ----------------------------------------------------------------------------
function switchAdminTab(subTab) {
    document.querySelectorAll('.admin-section').forEach(el => el.classList.add('hidden'));
    document.querySelectorAll('.admin-tab-btn').forEach(btn => {
        btn.classList.remove('border-blue-600', 'text-blue-600');
        btn.classList.add('border-transparent', 'text-slate-500');
    });

    document.getElementById(`admin-section-${subTab}`)?.classList.remove('hidden');
    const activeBtn = document.getElementById(`admin-tab-${subTab}`);
    if (activeBtn) {
        activeBtn.classList.remove('border-transparent', 'text-slate-500');
        activeBtn.classList.add('border-blue-600', 'text-blue-600');
    }

    if (subTab === 'trains') loadAdminTrains();
    if (subTab === 'stations') loadAdminStations();
    if (subTab === 'bookings') loadAdminBookings();
    if (subTab === 'audit') loadAdminAuditLogs();
}

async function loadAdminDashboard() {
    try {
        const res = await fetch(`${API_BASE}/admin/stats`);
        const stats = await res.json();

        document.getElementById('stat-bookings').textContent = stats.totalBookings;
        document.getElementById('stat-revenue').textContent = `Rs. ${stats.netRevenueRupees.toLocaleString('en-IN')}`;
        document.getElementById('stat-trains').textContent = stats.totalTrains;
        document.getElementById('stat-stations').textContent = stats.totalStations;

        loadAdminTrains();
    } catch (err) {
        console.error('Failed to load stats:', err);
    }
}

async function loadAdminTrains() {
    const tbody = document.getElementById('admin-trains-table');
    tbody.innerHTML = `<tr><td colspan="6" class="p-6 text-center text-slate-500"><i class="fa-solid fa-spinner fa-spin"></i></td></tr>`;

    try {
        const res = await fetch(`${API_BASE}/trains`);
        const trains = await res.json();

        tbody.innerHTML = trains.map(t => `
            <tr>
                <td class="p-3 font-mono font-bold text-blue-900">${t.train_number}</td>
                <td class="p-3 font-semibold text-slate-800">${escapeHtml(t.name)}</td>
                <td class="p-3"><span class="px-2 py-0.5 rounded text-xs font-bold uppercase bg-slate-100">${t.type}</span></td>
                <td class="p-3 text-xs text-slate-600">${t.source_code} &rarr; ${t.dest_code}</td>
                <td class="p-3 text-xs font-semibold">Rs. ${t.base_fare_per_km.toFixed(2)}</td>
                <td class="p-3">
                    <span class="px-2 py-0.5 rounded-full text-xs font-bold ${t.is_active ? 'bg-emerald-100 text-emerald-800' : 'bg-slate-200 text-slate-600'}">
                        ${t.is_active ? 'ACTIVE' : 'INACTIVE'}
                    </span>
                </td>
            </tr>
        `).join('');
    } catch (err) {
        tbody.innerHTML = `<tr><td colspan="6" class="p-4 text-rose-600">${err.message}</td></tr>`;
    }
}

async function loadAdminStations() {
    const tbody = document.getElementById('admin-stations-table');
    tbody.innerHTML = `<tr><td colspan="6" class="p-6 text-center text-slate-500"><i class="fa-solid fa-spinner fa-spin"></i></td></tr>`;

    try {
        const res = await fetch(`${API_BASE}/stations?sort=asc`);
        const stations = await res.json();

        tbody.innerHTML = stations.map(s => `
            <tr>
                <td class="p-3 font-mono font-bold text-blue-900">${s.code}</td>
                <td class="p-3 font-semibold text-slate-800">${escapeHtml(s.name)}</td>
                <td class="p-3 text-slate-600 font-medium">${s.district}</td>
                <td class="p-3 text-xs text-slate-500">${s.railway_division} Div (${s.railway_zone})</td>
                <td class="p-3 text-slate-700 font-semibold text-center">${s.platforms_count}</td>
                <td class="p-3 text-right">
                    <button onclick="deleteStationFromAdmin(${s.station_id}, '${s.code}')" class="px-2.5 py-1 text-xs bg-rose-50 hover:bg-rose-100 text-rose-700 border border-rose-200 rounded-lg transition font-semibold">
                        <i class="fa-solid fa-trash mr-1"></i> Delete
                    </button>
                </td>
            </tr>
        `).join('');
    } catch (err) {
        tbody.innerHTML = `<tr><td colspan="6" class="p-4 text-rose-600">${err.message}</td></tr>`;
    }
}

async function deleteStationFromAdmin(stationId, stationCode) {
    if (!confirm(`Are you sure you want to delete Karnataka station [${stationCode}]? The system will verify that it is not referenced by active routes or bookings.`)) {
        return;
    }

    try {
        const res = await fetch(`${API_BASE}/stations/${stationId}?adminUserId=${currentUser ? currentUser.userId : 1}`, {
            method: 'DELETE'
        });
        const data = await res.json();
        if (!res.ok) throw new Error(data.error || 'Failed to delete station');

        showToast(`Station ${stationCode} deleted successfully.`);
        loadStations();
        loadAdminStations();
    } catch (err) {
        alert(err.message);
        showToast(err.message, 'error');
    }
}

async function loadAdminBookings() {
    const tbody = document.getElementById('admin-bookings-table');
    tbody.innerHTML = `<tr><td colspan="8" class="p-6 text-center text-slate-500"><i class="fa-solid fa-spinner fa-spin"></i></td></tr>`;

    try {
        const res = await fetch(`${API_BASE}/admin/bookings`);
        const bookings = await res.json();

        tbody.innerHTML = bookings.map(b => `
            <tr>
                <td class="p-3 font-mono font-bold text-blue-900">${b.pnr}</td>
                <td class="p-3 text-xs text-slate-700">${b.username} (${b.full_name})</td>
                <td class="p-3 text-xs font-semibold">#${b.train_number} - ${b.train_name}</td>
                <td class="p-3 text-xs text-slate-600">${b.journey_date}</td>
                <td class="p-3 text-xs font-bold">${b.coach_class}</td>
                <td class="p-3 text-xs text-slate-600">${b.passengers.length} pax</td>
                <td class="p-3 text-xs font-bold text-slate-900">Rs. ${b.totalFareRupees.toFixed(2)}</td>
                <td class="p-3">
                    <span class="px-2 py-0.5 rounded-full text-[10px] font-bold ${b.status === 'CONFIRMED' ? 'bg-emerald-100 text-emerald-800' : 'bg-rose-100 text-rose-800'}">
                        ${b.status}
                    </span>
                </td>
            </tr>
        `).join('');
    } catch (err) {
        tbody.innerHTML = `<tr><td colspan="8" class="p-4 text-rose-600">${err.message}</td></tr>`;
    }
}

async function loadAdminAuditLogs() {
    const tbody = document.getElementById('admin-audit-table');
    tbody.innerHTML = `<tr><td colspan="5" class="p-6 text-center text-slate-500"><i class="fa-solid fa-spinner fa-spin"></i></td></tr>`;

    try {
        const res = await fetch(`${API_BASE}/admin/audit-logs`);
        const logs = await res.json();

        tbody.innerHTML = logs.map(l => `
            <tr>
                <td class="p-3 text-slate-400">${l.timestamp}</td>
                <td class="p-3 font-semibold text-slate-700">${l.username || 'System'}</td>
                <td class="p-3 font-bold text-blue-700">${l.action}</td>
                <td class="p-3 text-slate-500">${l.entity_name} (#${l.entity_id})</td>
                <td class="p-3 text-slate-600">${escapeHtml(l.details || '')}</td>
            </tr>
        `).join('');
    } catch (err) {
        tbody.innerHTML = `<tr><td colspan="5" class="p-4 text-rose-600">${err.message}</td></tr>`;
    }
}

function openAddTrainModal() {
    const number = prompt("Enter Karnataka Train Number (e.g. 16535):");
    if (!number) return;
    const name = prompt("Enter Train Name (e.g. Golgumbaz Express):");
    if (!name) return;
    const type = prompt("Enter Train Type (LOCAL, EXPRESS, SUPERFAST, SLEEPER, RAJDHANI, VANDE_BHARAT):", "EXPRESS");
    if (!type) return;

    fetch(`${API_BASE}/trains`, {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({
            train_number: number,
            name,
            type,
            source_station_id: 1,  // SBC
            dest_station_id: 52, // BGK
            route_id: 2,
            base_fare_per_km: 0.55,
            is_active: true,
            adminUserId: currentUser ? currentUser.userId : 1
        })
    }).then(r => r.json()).then(data => {
        if (data.error) throw new Error(data.error);
        showToast(`Train #${number} created successfully.`);
        loadAdminTrains();
    }).catch(e => showToast(e.message, 'error'));
}

function openAddStationModal() {
    const code = prompt("Enter Official Karnataka Station Code (e.g. BYPL, DVG):");
    if (!code) return;
    const name = prompt("Enter Station Name (e.g. Baiyyappanahalli):");
    if (!name) return;
    const city = prompt("Enter City / Town:", "Bengaluru");
    const district = prompt("Enter Karnataka District:", "Bengaluru Urban");
    const division = prompt("Enter Railway Division (Bengaluru, Mysuru, Hubballi, Karwar, Palakkad, Solapur):", "Bengaluru");
    const zone = prompt("Enter Railway Zone (South Western Railway, Konkan Railway, Southern Railway, Central Railway):", "South Western Railway");

    fetch(`${API_BASE}/stations`, {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({
            code: code.trim().toUpperCase(),
            name: name.trim(),
            city: city.trim(),
            district: district.trim(),
            state: "Karnataka",
            railway_zone: zone.trim(),
            railway_division: division.trim(),
            platforms_count: 4,
            adminUserId: currentUser ? currentUser.userId : 1
        })
    }).then(r => r.json()).then(data => {
        if (data.error) throw new Error(data.error);
        showToast(`Station ${code} added to Karnataka Railway Database!`);
        loadStations();
        loadAdminStations();
    }).catch(e => showToast(e.message, 'error'));
}

// Utility
function escapeHtml(str) {
    if (!str) return '';
    return String(str)
        .replace(/&/g, '&amp;')
        .replace(/</g, '&lt;')
        .replace(/>/g, '&gt;')
        .replace(/"/g, '&quot;')
        .replace(/'/g, '&#039;');
}
