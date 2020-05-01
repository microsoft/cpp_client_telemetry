// Copyright (C) Microsoft Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

'use strict';

var i = 0;
var loadTimeData = {
  getString: function (param) {
    i++;
    return param + i;
  }
};

document.addEventListener('DOMContentLoaded', () => {
  // Query data from backend
  const dataModel = {
    // Must match DataViewerRequest in
    // chrome/browser/ui/webui/edge_data_viewer/edge_data_viewer_ui.cc
    id_to_enum: {
      'settings': '1',
      'event-list': '2',
      'count-new-events': '3',
      'payload-view': '4',
    },

    base_events: window.baseEvents,

    category_list: {
      'names': [
        loadTimeData.getString('browsingHistory'),
        loadTimeData.getString('deviceConnectivityAndConfiguration'),
        loadTimeData.getString('inkingTypingAndSpeechUtterance'),
        loadTimeData.getString('productAndServicePerformance'),
        loadTimeData.getString('productAndServiceUsage'),
        loadTimeData.getString('softwareSetupAndInventory'),
      ],
      'tags': {
        'Privacy.DataType.BrowsingHistory': 0,
        'Privacy.DataType.DeviceConnectivityAndConfiguration': 1,
        'Privacy.DataType.InkingTypingAndSpeechUtterance': 2,
        'Privacy.DataType.ProductAndServicePerformance': 3,
        'Privacy.DataType.ProductAndServiceUsage': 4,
        'Privacy.DataType.SoftwareSetupAndInventory': 5,
      },
      'events': {},
    },

    init: () => {
      dataModel.initCategoryList();
    },

    initCategoryList: () => {
      Object.assign(dataModel, window.dataModelSource);
      /*
      for (const provider of dataModel.base_events['Providers']) {
        for (const event of provider['Events']) {
          const tags = [];
          for (const tag of event['PrivacyTags']['AssetTags']) {
            const indexTag = dataModel.category_list.tags[tag];
            if (!isNaN(indexTag)) {
              tags.push(indexTag);
            }
          }
          const name = provider['Name'] + '.' + event['Name'];
          dataModel.category_list.events[name] = tags;
        }
      }
      */
    },

    requestHtml: (id, param, done) => {
      cr.sendWithPromise(
        'requestDataViewerData', dataModel.id_to_enum[id], param)
      .then((html) => {
        dataModel.onHtml(id, html, done);
      });
    },

    requestJson: (id, param, done) => {
      var j = { "name":"Max", "age":41, "car":"Honda" };
      dataModel.onJson(id, j, done);
/*
      cr.sendWithPromise(
        'requestDataViewerData', dataModel.id_to_enum[id], param)
      .then((json) => {
        dataModel.onJson(id, json, done);
      });
*/
    },

    onHtml: (id, html, done) => {
      const target = $(id);
      if (target) {
        target.innerHTML = html;
      }
      if (done) {
        done();
      }
    },

    onJson: (id, json, done) => {
      let obj = null;
      try {
        obj = JSON.parse(json);
      } catch (err) {
        obj = {};
      }
      done(obj);
    },
  };

  // Category Filter
  const categoryFilterController = {
    init: () => {
      categoryFilterController.categoryFilterMenu.init();
      categoryFilterController.filterButton.init();
    },

    getFilter: () => {
      return categoryFilterController.categoryFilterMenu.getSelection();
    },

    toggleFilterMenu: () => {
      categoryFilterController.categoryFilterMenu.toggle();
    },

    closeFilterMenu: () => {
      categoryFilterController.categoryFilterMenu.close();
    },

    categoryFilterMenu: {
      id: 'filter-menu-category-list',

      init: () => {
        categoryFilterController.categoryFilterMenu.showMenuItems();
        categoryFilterController.categoryFilterMenu.overlay.init();
        for (const item of $(categoryFilterController.categoryFilterMenu.id)
          .closest('.filter-menu').getElementsByClassName('menu-item')) {
          item.onkeyup = categoryFilterController.categoryFilterMenu.onKeyUp;
        }
      },

      showMenuItems: () => {
        let html = '';
        const tabindexStart = 10;
        for (let i = 0;
          i < dataModel.category_list.names.length;
          i++) {
            html += `<div class="menu-item toggle focusable" data-index="`;
            html += i;
            html += `" role="menuitemcheckbox" data-tabindex="`;
            html += (i + tabindexStart);
            html += `">`;
            html += dataModel.category_list.names[i];
            html += `</div>`;
        }
        $(categoryFilterController.categoryFilterMenu.id).innerHTML = html;
      },

      toggle: () => {
        $(categoryFilterController.categoryFilterMenu.overlay.id).classList
        .toggle('hidden-overlay');
        keyboardInputController.updateTabIndex();
        categoryFilterController.categoryFilterMenu.focusMenuItem();
      },

      isOpen: () => {
        return !$(categoryFilterController.categoryFilterMenu.overlay.id)
          .classList.contains('hidden-overlay');
      },

      close: () => {
        $(categoryFilterController.categoryFilterMenu.overlay.id).classList
        .add('hidden-overlay');
        keyboardInputController.updateTabIndex();
      },

      focusMenuItem: () => {
        if (categoryFilterController.categoryFilterMenu.isOpen()) {
          keyboardInputController.focusNext(
            $(categoryFilterController.filterButton.id), 'menu-item');
        }
      },

      toggleMenuItem: (menu_item) => {
        if (menu_item.classList.contains('toggle')) {
          menu_item.classList.toggle('selected');
          menu_item.setAttribute(
            'aria-checked', menu_item.classList.contains('selected'));
        }
        if (menu_item.classList.contains('clear-toggles')) {
          categoryFilterController.categoryFilterMenu.clearSelection();
        }
        if (menu_item.id ===
          categoryFilterController.categoryFilterMenu.overlay.id) {
          categoryFilterController.closeFilterMenu();
        }
        categoryFilterController.filterButton.updateIcon(
          categoryFilterController.categoryFilterMenu.isFiltered());
        eventListController.refresh();
      },

      getSelection: () => {
        const filteredCategories = {};
        const toggles = $(categoryFilterController.categoryFilterMenu.id)
        .getElementsByClassName('toggle');
        for (let i = 0; i < toggles.length; i++) {
          if (toggles[i].classList.contains('selected')) {
            filteredCategories[+toggles[i].getAttribute('data-index')] = true;
          } else {
            filteredCategories[+toggles[i].getAttribute('data-index')] = false;
          }
        }
        return filteredCategories;
      },

      clearSelection: () => {
        const toggles = $(categoryFilterController.categoryFilterMenu.id)
        .getElementsByClassName('toggle');
        for (let i = 0; i < toggles.length; i++) {
          toggles[i].classList.remove('selected');
          toggles[i].removeAttribute('aria-checked');
        }
      },

      isFiltered: () => {
        const toggles = $(categoryFilterController.categoryFilterMenu.id)
        .getElementsByClassName('toggle');
        for (let i = 0; i < toggles.length; i++) {
          if (toggles[i].classList.contains('selected')) {
            return true;
          }
        }
        return false;
      },

      onKeyUp: (event) => {
        switch (event.key) {
          case 'Enter':
          case ' ':
            categoryFilterController.categoryFilterMenu.toggleMenuItem(
              event.target);
            break;
          case 'ArrowDown':
            keyboardInputController.focusNext(event.target, 'menu-item');
            break;
          case 'ArrowUp':
            keyboardInputController.focusPrev(event.target, 'menu-item');
            break;
          case 'Escape':
            categoryFilterController.closeFilterMenu();
            $(categoryFilterController.filterButton.id).focus();
            break;
        }
      },

      overlay: {
        id: 'filter-menu-overlay',

        init: () => {
          $(categoryFilterController.categoryFilterMenu.overlay.id).onclick =
            categoryFilterController.categoryFilterMenu.overlay.onClick;
        },

        onClick: (event) => {
          categoryFilterController.categoryFilterMenu.toggleMenuItem(
            event.target);
        },
      },
    },

    filterButton: {
      id: 'filter-icon',

      init: () => {
        const button = $(categoryFilterController.filterButton.id);
        button.onclick = categoryFilterController.filterButton.onClick;
        button.onkeyup = categoryFilterController.filterButton.onKeyUp;
      },

      updateIcon: (filtered) => {
        if (filtered) {
          $('filter-icon').classList.add('filtered');
        } else {
          $('filter-icon').classList.remove('filtered');
        }
      },

      onClick: () => {
        categoryFilterController.toggleFilterMenu();
      },

      onKeyUp: (event) => {
        switch (event.key) {
          case 'Enter':
          case ' ':
            categoryFilterController.filterButton.onClick();
            break;
          case 'ArrowUp':
            categoryFilterController.closeFilterMenu();
            break;
          case 'ArrowDown':
            if (categoryFilterController.categoryFilterMenu.isOpen()) {
              keyboardInputController.focusNext(event.target, 'menu-item');
            } else {
              categoryFilterController.filterButton.onClick();
            }
            break;
        }
      },
    },
  };

  // Category List (on event in current view)
  const categoryListController = {
    showCategoryList: (event) => {
      categoryListController.categoryListView.show(event);
    },

    categoryListView: {
      id: 'event-category-list',

      show: (event) => {
        if (event) {
          const tags = dataModel.category_list.events[
            event.getElementsByClassName('event-name')[0].textContent];
          let html = '';
          if (Array.isArray(tags)) {
            for (let i = 0; i < tags.length; i++) {
              html += `<div class="event-category" data-index="`;
              html += tags[i];
              html += `" role="option">`;
              html += dataModel.category_list.names[tags[i]];
              html += `</div>`;
            }
          }
          $(categoryListController.categoryListView.id).innerHTML = html;
          categoryListController.categoryListView.update(
            categoryFilterController.getFilter());
          keyboardInputController.updateTabIndex();
        } else {
          $(categoryListController.categoryListView.id).innerHTML = '';
        }
        if ($(categoryListController.categoryListView.id).innerHTML) {
          $('payload-view-container').classList.remove('no-category');
          $(categoryListController.categoryListView.id).classList
          .remove('no-category');
        } else {
          $('payload-view-container').classList.add('no-category');
          $(categoryListController.categoryListView.id).classList
          .add('no-category');
        }
      },

      update: (filteredCategories) => {
        const categories = $(categoryListController.categoryListView.id)
        .getElementsByClassName('event-category');
        for (let i = 0; i < categories.length; i++) {
          if (filteredCategories[categories[i].getAttribute('data-index')]) {
            categories[i].classList.add('filtered');
            categories[i].setAttribute('aria-selected', 'true');
          } else {
            categories[i].classList.remove('filtered');
            categories[i].setAttribute('aria-selected', 'false');
          }
        }
      },
    },
  };

  // Event List
  const eventListController = {
    init: () => {
      eventListController.eventListView.init();
      eventListController.refreshButton.init();
    },

    refresh: () => {
      eventListController.refreshButton.refresh();
    },

    onKeyDown: (event) => {
      eventListController.eventListView.onKeyDown(event);
    },

    eventListView: {
      id: 'event-list',

      init: () => {
        const list = $(eventListController.eventListView.id);
        list.onmousedown = eventListController.eventListView.onMouseDown;
        list.onclick = eventListController.eventListView.onClick;
        list.onkeydown = eventListController.eventListView.onKeyDown;
        eventListController.eventListView.observer.init();
      },

      showNewEvents: () => {
        dataModel.requestHtml(eventListController.eventListView.id,
          null, eventListController.eventListView.onShow);
      },

      filterNewEvents: () => {
        const isCategoryFiltered =
          categoryFilterController.categoryFilterMenu.isFiltered();
        let filteredCategories = {};
        if (isCategoryFiltered) {
          filteredCategories = categoryFilterController.getFilter();
        }

        const nameFilter = nameFilterController.getFilter();
        let regex = null;
        if (nameFilter) {
          regex = new RegExp(nameFilter);
        }

        const allEvents = $(eventListController.eventListView.id)
        .getElementsByClassName('event');
        for (let i = 0; i < allEvents.length; i++) {
          allEvents[i].classList.add('visible-event');
          // Filter by name
          const name = allEvents[i].getElementsByClassName(
            'event-name')[0].textContent;
          if (regex && !regex.test(name)) {
            allEvents[i].classList.add('hidden-event');
            allEvents[i].classList.remove('visible-event');
            continue;
          }
          // Filter by category
          const tags = dataModel.category_list['events'][name];
          if (isCategoryFiltered) {
            let selected = false;
            if (Array.isArray(tags)) {
              for (let j = 0; j < tags.length; j++) {
                if (filteredCategories[tags[j]]) {
                  selected = true;
                  break;
                }
              }
            }
            if (!selected) {
              allEvents[i].classList.add('hidden-event');
              allEvents[i].classList.remove('visible-event');
            }
          }
        }
      },

      scrollToTop: () => {
        $(eventListController.eventListView.id).scrollTop = 0;
      },

      select: (index) => {
        if (index >= 0) {
          const allEvents = $(eventListController.eventListView.id)
          .getElementsByClassName('visible-event');
          if (index < allEvents.length) {
            eventListController.eventListView.onSelect(allEvents[index]);
          }
        }
      },

      focus: (event) => {
        const tabIndex = +$(eventListController.eventListView.id)
        .getAttribute('tabindex');
        event.setAttribute('tabindex', tabIndex);
        event.setAttribute('aria-selected', 'true');
        event.focus();
      },

      countVisibleEvents: () => {
        let count = 0;
        const allEvents = $(eventListController.eventListView.id)
        .getElementsByClassName('event');
        for (const event of allEvents) {
          if (event.classList.contains('visible-event')) {
            count++;
          }
        }
        return count;
      },

      notify: () => {
        const notification = $('notification');
        const eventListView = $(eventListController.eventListView.id);
        const count = eventListController.eventListView.countVisibleEvents();
        if (count == 0 && eventListView.getAttribute('data-initial-load')) {
          if (pageController.settings.is_uma_enabled) {
            notification.innerHTML = loadTimeData.getString(
              'messageUmaEnabled');
          } else {
            notification.innerHTML = loadTimeData.getString(
              'messageUmaDisabled');
          }
        } else {
          notification.innerHTML = loadTimeData.getString(
            'messageEventsListed');
          $('count-events-listed').textContent = count;
        }
        eventListView.removeAttribute('data-initial-load');
      },

      onBlur: (event) => {
        event.removeAttribute('tabindex');
        event.removeAttribute('aria-selected');
      },

      onShow: () => {
        eventListController.eventListView.observer.unobserve();
        eventListController.eventListView.filterNewEvents();
        eventListController.eventListView.scrollToTop();
        eventListController.eventListView.select(0);
        eventListController.eventListView.notify();
      },

      onMouseDown: (event) => {
        const closest = event.target.closest('.event');
        if (closest) {
          eventListController.eventListView.event_mouse_down = closest;
        }
      },

      onClick: (event) => {
        const closest = event.target.closest('.event');
        eventListController.eventListView.onSelect(closest ? closest :
          eventListController.eventListView.event_mouse_down);
      },

      onSelect: (event) => {
        if (event && !event.classList.contains('selected')) {
          if (eventListController.eventListView.event_selected) {
            eventListController.eventListView.event_selected.classList.remove(
              'selected');
          }
          event.classList.add('selected');
          payloadController.showPayload(event);
          categoryListController.showCategoryList(event);
          eventListController.eventListView.observer.observe(event);
        }
      },

      onKeyDown: (event) => {
        let sibling;
        switch (event.key) {
          case 'ArrowDown':
            sibling = eventListController.eventListView.event_selected;
            do {
              sibling = sibling && sibling.nextSibling;
            }
            while (sibling && sibling.classList.contains('hidden-event'));
            break;
          case 'ArrowUp':
            sibling = eventListController.eventListView.event_selected;
            do {
              sibling = sibling && sibling.previousSibling;
            }
            while (sibling && sibling.classList.contains('hidden-event'));
            break;
          case 'Enter':
          case ' ':
              if (eventListController.eventListView.event_selected) {
                eventListController.eventListView.focus(
                  eventListController.eventListView.event_selected);
              }
              event.preventDefault();
              break;
        }
        if (sibling) {
          eventListController.eventListView.focus(sibling);
          eventListController.eventListView.onSelect(sibling);
          if (eventListController.eventListView.observer.disableScroll(
            event.key)) {
            event.preventDefault();
          }
        }
      },

      observer: {
        instance: null,

        init: () => {
          eventListController.eventListView.observer.instance =
            new IntersectionObserver(
              eventListController.eventListView.observer.onIntersection, {
                root: $(eventListController.eventListView.id),
                rootMargin: '-60px',
                threshold: 0,
              });
        },

        observe: (event) => {
          eventListController.eventListView.observer.unobserve();
          eventListController.eventListView.observer.instance.observe(event);
          eventListController.eventListView.event_selected = event;
        },

        unobserve: () => {
          if (eventListController.eventListView.event_selected) {
            eventListController.eventListView.observer.instance.unobserve(
              eventListController.eventListView.event_selected);
            eventListController.eventListView.onBlur(
              eventListController.eventListView.event_selected);
            eventListController.eventListView.event_selected = null;
          }
        },

        disableScroll: (key) => {
          const entry = eventListController.eventListView.observer.entry;
          if (entry.isIntersecting) {
            return true;
          }
          const upperHalf = (entry.boundingClientRect.top +
            entry.boundingClientRect.height / 2) < (entry.rootBounds.top +
            entry.rootBounds.height / 2);
          if (upperHalf && key == 'ArrowDown' ||
            !upperHalf && key == 'ArrowUp') {
            return true;
          }
          return false;
        },

        onIntersection: (entries) => {
          for (const entry of entries) {
            eventListController.eventListView.observer.entry = entry;
          }
        },

        entry: {},
      },

      event_mouse_down: null,
      event_selected: null,
    },

    refreshButton: {
      id: 'refresh-icon',
      msecStart: 0,
      secTimeout: 10,

      init: () => {
        const button = $(eventListController.refreshButton.id);
        button.onclick = eventListController.refreshButton.onClick;
        button.onkeyup = eventListController.refreshButton.onKeyUp;
        eventListController.refreshButton.msecStart = Date.now();
        window.requestAnimationFrame(
          eventListController.refreshButton.onFrame);
      },

      refresh: () => {
        eventListController.refreshButton.msecStart = Date.now();
        eventListController.refreshButton.resetNewEvents();
        payloadController.showPayload(null);
        categoryListController.showCategoryList(null);
        eventListController.eventListView.showNewEvents();
      },

      countNewEvents: () => {
        const countNewEvents = $('count-new-events');
        return countNewEvents ? +countNewEvents.textContent : 0;
      },

      resetNewEvents: () => {
        $('notification').textContent = '';
        $(eventListController.refreshButton.id).classList.remove(
          'new-events');
      },

      onFrame: () => {
        const sec = (Date.now() - eventListController.refreshButton.msecStart)
          / 1000 | 0;
        if (sec >= eventListController.refreshButton.secTimeout) {
          eventListController.refreshButton.msecStart = Date.now();
          dataModel.requestJson('count-new-events', null, (obj) => {
            if (obj) {
              eventListController.refreshButton.onNewEvents(+obj.count);
            }
          });
        }
        window.requestAnimationFrame(
          eventListController.refreshButton.onFrame);
      },

      onNewEvents: (count) => {
        if (count > eventListController.refreshButton.countNewEvents()) {
          $('notification').innerHTML = loadTimeData.getString(
            'messageNewEvents');
          $('count-new-events').textContent = count;
          $(eventListController.refreshButton.id).classList.add(
            'new-events');
        }
      },

      onClick: () => {
        eventListController.refreshButton.refresh();
        categoryFilterController.closeFilterMenu();
      },

      onKeyUp: (event) => {
        if (event.key === 'Enter' || event.key === ' ') {
          eventListController.refreshButton.onClick();
        }
      },
    },
  };

  // Keyboard Input
  const keyboardInputController = {
    show_focus_outline: false,

    init: () => {
      document.body.onkeydown = keyboardInputController.onKeyDown;
      document.body.onkeyup = keyboardInputController.onKeyUp;
      document.body.onmousedown = keyboardInputController.onMouseDown;
    },

    updateTabIndex: () => {
      if (keyboardInputController.show_focus_outline) {
        keyboardInputController.addTabIndex(document);
        const overlay =
          $(categoryFilterController.categoryFilterMenu.overlay.id);
        if (overlay.classList.contains('hidden-overlay')) {
          keyboardInputController.removeTabIndex(overlay);
        }
      } else {
        keyboardInputController.removeTabIndex(document);
      }
    },

    addTabIndex: (parent) => {
      for (const focusable of parent.getElementsByClassName('focusable')) {
        focusable.setAttribute(
          'tabindex', +focusable.getAttribute('data-tabindex'));
        focusable.classList.add('has-tabindex');
      }
    },

    removeTabIndex: (parent) => {
      for (const focusable of parent.getElementsByClassName('focusable')) {
        focusable.removeAttribute('tabindex');
        focusable.classList.remove('has-tabindex');
      }
    },

    focusNext: (target, className) => {
      const current = +target.getAttribute('tabindex');
      let move;
      for (const focusable of document.getElementsByClassName('focusable')) {
        if (focusable.classList.contains(className)) {
          const tabindex = +focusable.getAttribute('tabindex');
          if (tabindex > current) {
            move = focusable;
            break;
          }
        }
      }
      if (move) {
        move.focus();
      }
    },

    focusPrev: (target, className) => {
      const current = +target.getAttribute('tabindex');
      let move;
      for (const focusable of document.getElementsByClassName('focusable')) {
        if (focusable.classList.contains(className)) {
          const tabindex = +focusable.getAttribute('tabindex');
          if (tabindex >= current) {
            break;
          } else if (tabindex > 0) {
            move = focusable;
          }
        }
      }
      if (move) {
        move.focus();
      }
    },

    onKeyDown: (event) => {
      if (!keyboardInputController.show_focus_outline) {
        keyboardInputController.show_focus_outline = true;
        keyboardInputController.updateTabIndex();
      }
      if (document.activeElement === document.body) {
        switch (event.key) {
          case 'ArrowDown':
          case 'ArrowUp':
          case 'Enter':
          case ' ':
            if (categoryFilterController.categoryFilterMenu.isOpen()) {
              $(categoryFilterController.filterButton.id).focus();
            } else {
              $(eventListController.eventListView.id).focus();
              eventListController.onKeyDown(event);
            }
            break;
        }
      }
    },

    onKeyUp: (event) => {
      switch (event.key) {
        case 'Escape':
          categoryFilterController.closeFilterMenu();
          break;
      }
    },

    onMouseDown: (event) => {
      if (event.target.closest('.payload-view-header')) {
        categoryFilterController.closeFilterMenu();
      }
      if (keyboardInputController.show_focus_outline) {
        keyboardInputController.show_focus_outline = false;
        keyboardInputController.updateTabIndex();
      }
    },
  };

  // Event Name Filter
  const nameFilterController = {
    init: () => {
      nameFilterController.nameFilterInput.init();
    },

    getFilter: () => {
      return $(nameFilterController.nameFilterInput.id).value;
    },

    nameFilterInput: {
      id: 'event-name-filter',

      init: () => {
        const input = $(nameFilterController.nameFilterInput.id);
        input.onclick = nameFilterController.nameFilterInput.onClick;
        input.onkeyup = nameFilterController.nameFilterInput.onKeyUp;
      },

      onClick: () => {
        categoryFilterController.closeFilterMenu();
      },

      onKeyUp: (event) => {
        if (event.key === 'Enter') {
          eventListController.refresh();
        }
      },
    },
  };

  // Main Page
  const pageController = {
    id: 'page',

    settings: {
      'should_use_utc': false,
      'not_registered': false,
      'is_uma_enabled': false,
    },

    init: (onReady) => {
      pageController.initLocalizedStrings();
      dataModel.requestJson('settings', null, (obj) => {
        pageController.settings = obj;
        pageController.show();
        if (pageController.isReady()) {
          onReady();
        }
      });
    },

    initLocalizedStrings: () => {
      $('should-use-utc').innerHTML = loadTimeData.getString('shouldUseUtc');
    },

    isReady: () => {
      if (pageController.settings.should_use_utc) {
        return false;
      }
      if (pageController.settings.not_registered) {
        return false;
      }
      return true;
    },

    show: () => {
      if (pageController.settings.should_use_utc) {
        $('should-use-utc').classList.remove('hidden-message');
      }
      if (pageController.settings.not_registered) {
        $('not-registered').classList.remove('hidden-message');
      }
      if (pageController.isReady()) {
        $(pageController.id).classList.remove('hidden-page');
      }
    },
  };

  // Event Payload View
  const payloadController = {
    showPayload: (event) => {
      payloadController.payloadView.show(event);
    },

    payloadView: {
      id: 'payload-view',

      show: (event) => {
        if (event) {
          dataModel.requestHtml(payloadController.payloadView.id,
            event.getAttribute('data-index'),
            () => {
              if (!$(payloadController.payloadView.id).innerHTML) {
                $(payloadController.payloadView.id).innerHTML =
                  loadTimeData.getString('noPayload');
              }
            }
          );
          payloadController.payloadView.scrollToTop();
        } else {
          $(payloadController.payloadView.id).innerHTML = '';
        }
      },

      scrollToTop: () => {
        $(payloadController.payloadView.id).scrollTop = 0;
      },
    },
  };

  // Init
  keyboardInputController.init();
  pageController.init(() => {
    dataModel.init();
    categoryFilterController.init();
    eventListController.init();
    nameFilterController.init();

    // Test hook
    window.refreshDataViewer = () => {
      eventListController.refresh();
      return true;
    };

    // Initial refresh
    refreshDataViewer();
  });
});
